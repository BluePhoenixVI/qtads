// This is copyrighted software. More information is at the end of this file.
#include <QDebug>
#include <QFileInfo>
#include <QResource>

#ifndef NO_AUDIO
    #include "Aulib/DecoderFluidsynth.h"
    #include "Aulib/DecoderMpg123.h"
    #include "Aulib/DecoderSndfile.h"
    #include "Aulib/DecoderVorbis.h"
    #include "Aulib/ResamplerSpeex.h"
    #include "aulib.h"
    #include "rwopsbundle.h"
    #include <SDL.h>
    #include <SDL_error.h>
    #include <SDL_rwops.h>
    #include <cerrno>
    #include <cstdio>
    #include <cstring>
    #include <mutex>
#endif

#include "globals.h"
#include "qtadssound.h"
#include "qtimerchrono.h"
#include "settings.h"
#include "sysframe.h"
#include "syssoundmidi.h"
#include "syssoundmpeg.h"
#include "syssoundogg.h"
#include "syssoundwav.h"

namespace chrono = std::chrono;
using namespace std::chrono_literals;

#ifndef NO_AUDIO
static std::vector<Aulib::Stream*> streamsPendingDeletion;
static std::mutex streamsPendingDeletion_mutex;
#endif

auto initSound() -> bool
{
#ifndef NO_AUDIO
    if (not Aulib::init(44100, AUDIO_S16SYS, 2, 2048)) {
        qWarning("Unable to initialize SDL_audiolib: %s", SDL_GetError());
        return false;
    }
    return true;
#else
    return false;
#endif
}

void quitSound()
{
#ifndef NO_AUDIO
    {
        std::lock_guard<std::mutex> guard(streamsPendingDeletion_mutex);
        for (auto* stream : streamsPendingDeletion) {
            delete stream;
        }
        streamsPendingDeletion.clear();
    }
    Aulib::quit();
    SDL_Quit();
#endif
}

#ifndef NO_AUDIO
QTadsSound::QTadsSound(QObject* parent, Aulib::Stream* stream, SoundType type)
    : QObject(parent)
    , fAudStream(stream)
    , fType(type)
    , fPlaying(false)
    , fCrossFade(false)
    , fDone_func(nullptr)
    , fDone_func_ctx(nullptr)
    , fRepeats(0)
    , fRepeatsWanted(1)
    , fLength(chrono::duration_cast<chrono::milliseconds>(stream->duration()))
{
    // Pretend that the sound is 30ms shorter than it really is in order to
    // compensate for wacky OS timers (Windows and low-ticks systems).
    if (fLength > 30ms) {
        fLength -= 30ms;
    } else {
        fLength = 0ms;
    }
    // qDebug() << "Sound length:" << fLength;
    connect(this, &QTadsSound::readyToFadeOut, this, &QTadsSound::fPrepareFadeOut);
    connect(&fFadeOutTimer, &QTimer::timeout, this, &QTadsSound::fDoFadeOut);

    // Make sure the timer only calls our fade-out slot *once* and then stops.
    fFadeOutTimer.setSingleShot(true);
}

QTadsSound::~QTadsSound()
{
    // qDebug() << Q_FUNC_INFO;
    fRepeatsWanted = -1;
    fAudStream->unsetFinishCallback();
    fAudStream->unsetLoopCallback();

    std::lock_guard<std::mutex> guard(streamsPendingDeletion_mutex);
    streamsPendingDeletion.push_back(fAudStream);
}

void QTadsSound::fFinishCallback(Aulib::Stream& strm)
{
    // qDebug() << Q_FUNC_INFO;

    // Invoke the TADS callback, if there is one.
    fPlaying = false;
    if (fDone_func) {
        fDone_func(fDone_func_ctx, fRepeats);
    }
}

void QTadsSound::fLoopCallback(Aulib::Stream& strm)
{
    // qDebug() << Q_FUNC_INFO;

    ++fRepeats;
    fTimePos.start();

    // If this is the last iteration and we have a fade-out, set the fade-out
    // timer.
    if (fFadeOut.count() > 0 and (fRepeatsWanted == -1 or fRepeats == fRepeatsWanted)) {
        // Clamp the interval to 0 (= now), in case the fade out time is larger than the sound
        // length.
        fFadeOutTimer.start(std::max(0ms, fLength - fFadeOut));
    }
}

void QTadsSound::fDoFadeOut()
{
    Q_ASSERT(fAudStream->isPlaying());

    // If we need to do a crossfade, call the TADS callback now.
    if (fCrossFade and fDone_func) {
        fDone_func(fDone_func_ctx, fRepeats);
        // Make sure our Stream callback won't call the TADS callback a second time.
        fDone_func = nullptr;
        fDone_func_ctx = nullptr;
    }
    fAudStream->stop(chrono::milliseconds(fFadeOut));
}

void QTadsSound::fPrepareFadeOut()
{
    fFadeOutTimer.start(fLength - fFadeOut);
}

auto QTadsSound::startPlaying(
    void (*done_func)(void*, int repeat_count), void* done_func_ctx, int repeat, int vol,
    int fadeIn, int fadeOut, bool crossFade) -> int
{
    // qDebug() << Q_FUNC_INFO;

    // Check if user disabled digital sound.
    if (not qFrame->settings().enableSoundEffects) {
        return 1;
    }

    Q_ASSERT(not fPlaying);

    // Adjust volume if it exceeds min/max levels.
    if (vol < 0) {
        vol = 0;
    } else if (vol > 100) {
        vol = 100;
    }

    // Set the volume level.
    fAudStream->setVolume(static_cast<float>(vol) / 100.f);

    if (repeat < 0) {
        repeat = 0;
    }
    fRepeatsWanted = repeat;
    bool playOk;
    if (fadeIn > 0) {
        playOk = fAudStream->play(repeat, chrono::milliseconds(fadeIn));
    } else {
        playOk = fAudStream->play(repeat);
    }
    if (not playOk) {
        qWarning() << "ERROR:" << SDL_GetError();
        SDL_ClearError();
        return 1;
    } else {
        fTimePos.start();
        fPlaying = true;
        fCrossFade = crossFade;
        fRepeats = 1;
        fDone_func = done_func;
        fDone_func_ctx = done_func_ctx;
        if (fadeOut > 0) {
            fFadeOut = chrono::milliseconds(fadeOut);
            if (repeat == 1) {
                // The sound should only be played once.  We need to set the
                // fade-out timer here since otherwise the sound won't get a
                // chance to fade-out.
                emit readyToFadeOut();
            }
        }
    }
    return 0;
}

void QTadsSound::cancelPlaying(bool sync, int fadeOut, bool fadeOutInBg)
{
    if (not fPlaying) {
        return;
    }

    fRepeatsWanted = -1;

    if (not sync and fadeOut > 0) {
        if (fadeOutInBg and fDone_func) {
            // We need to do fade-out in the background; call the TADS callback
            // now.
            fDone_func(fDone_func_ctx, fRepeats);
            // Make sure our SDL callback won't call the TADS callback a second
            // time.
            fDone_func = nullptr;
            fDone_func_ctx = nullptr;
        }
        fAudStream->stop(chrono::milliseconds(fadeOut));
    } else {
        fAudStream->stop();
        fFinishCallback(*fAudStream);
    }

    if (sync) {
        // The operation needs to be synchronous; wait for the sound to finish.
        while (fAudStream->isPlaying()) {
            SDL_Delay(10);
        }
    }
}

void QTadsSound::addCrossFade(int ms)
{
    fCrossFade = true;
    fFadeOut = chrono::milliseconds(ms);

    if (fPlaying) {
        fRepeatsWanted = -1;
        auto timeFromNow = fLength - fFadeOut - chrono::milliseconds(fTimePos.elapsed());
        if (timeFromNow < 1ms) {
            timeFromNow = 1ms;
            fFadeOut = fLength - chrono::milliseconds(fTimePos.elapsed());
        }
        fFadeOutTimer.start(timeFromNow);
    }
}

static void deletePendingStreams()
{
    std::lock_guard<std::mutex> guard(streamsPendingDeletion_mutex);
    const auto streamsCopy = streamsPendingDeletion;
    for (auto* stream : streamsCopy) {
        if (!stream->isPlaying()) {
            const auto pos = std::find(streamsPendingDeletion.begin(), streamsPendingDeletion.end(), stream);
            Q_ASSERT(pos != streamsPendingDeletion.end());
            streamsPendingDeletion.erase(pos);
            delete stream;
        }
    }
}
#endif

auto QTadsSound::createSound(
    const CHtmlUrl* /*url*/, const textchar_t* filename, unsigned long seekpos,
    unsigned long filesize, CHtmlSysWin*, SoundType type) -> CHtmlSysSound*
#ifndef NO_AUDIO
{
    // qDebug() << "Loading sound from" << filename << "offset:" << seekpos << "size:" << filesize
    //      << "url:" << url->get_url();

    deletePendingStreams();

    // Check if the file exists and is readable.
    QFileInfo inf(fnameToQStr(filename));
    if (not inf.exists() or not inf.isReadable()) {
        qWarning() << "ERROR:" << inf.filePath() << "doesn't exist or is unreadable";
        return nullptr;
    }

    // Open the file and seek to the specified position.
    FILE* file = std::fopen(inf.filePath().toLocal8Bit().constData(), "rb");
    if (file == nullptr) {
        int errtmp = errno;
        qWarning() << "ERROR: Can't open file" << inf.filePath() << " (" << std::strerror(errtmp)
                   << ")";
        return nullptr;
    }
    if (std::fseek(file, seekpos, SEEK_SET) < 0) {
        int errtmp = errno;
        qWarning() << "ERROR: Can't seek in file" << inf.filePath() << " (" << std::strerror(errtmp)
                   << ")";
        std::fclose(file);
        return nullptr;
    }

    // Create the RWops through which the data will be read later.
    SDL_RWops* rw = RWFromMediaBundle(file, filesize);
    if (rw == nullptr) {
        qWarning() << "ERROR:" << SDL_GetError();
        SDL_ClearError();
        std::fclose(file);
        return nullptr;
    }

    std::unique_ptr<Aulib::Decoder> decoder = nullptr;
    switch (type) {
    case MPEG:
        decoder = std::make_unique<Aulib::DecoderMpg123>();
        break;
    case OGG:
        decoder = std::make_unique<Aulib::DecoderVorbis>();
        break;
    case WAV:
        decoder = std::make_unique<Aulib::DecoderSndfile>();
        break;
    case MIDI: {
        decoder = std::make_unique<Aulib::DecoderFluidsynth>();
        QResource sf2Res(QStringLiteral(":/soundfont.sf2"));
        auto* sf2_rwops = SDL_RWFromConstMem(sf2Res.data(), sf2Res.size());
        auto fsynth = static_cast<Aulib::DecoderFluidsynth*>(decoder.get());
        fsynth->loadSoundfont(sf2_rwops);
        fsynth->setGain(0.6f);
        break;
    }
    }

    Aulib::Stream* stream =
        new Aulib::Stream(rw, std::move(decoder), std::make_unique<Aulib::ResamplerSpeex>(), true);
    if (not stream->open()) {
        qWarning() << "ERROR:" << SDL_GetError();
        SDL_ClearError();
        delete stream;
        return nullptr;
    }

    // Create the sound object.  It is *important* not to pass the CHtmlSysWin object as the parent
    // in the constructor; doing so would result in Qt deleting the sound object when the parent
    // object gets destroyed.  Therefore, we simply pass 0 to make the sound object parentless.
    QTadsSound* sound = nullptr;
    switch (type) {
    case WAV:
        // qDebug() << "Sound type: WAV";
        sound = new CHtmlSysSoundWavQt(nullptr, stream, WAV);
        break;

    case OGG:
        // qDebug() << "Sound type: OGG";
        sound = new CHtmlSysSoundOggQt(nullptr, stream, OGG);
        break;

    case MPEG:
        // qDebug() << "Sound type: MPEG";
        sound = new CHtmlSysSoundMpegQt(nullptr, stream, MPEG);
        break;

    case MIDI:
        // qDebug() << "Sound type: MIDI";
        sound = new CHtmlSysSoundMidiQt(nullptr, stream, MIDI);
        break;
    }
    stream->setFinishCallback([sound](Aulib::Stream& strm) { sound->fFinishCallback(strm); });
    stream->setLoopCallback([sound](Aulib::Stream& strm) { sound->fLoopCallback(strm); });
    return dynamic_cast<CHtmlSysSound*>(sound);
}
#else
{
    return 0;
}
#endif

/*
    Copyright 2003-2020 Nikos Chantziaras <realnc@gmail.com>

    This file is part of QTads.

    QTads is free software: you can redistribute it and/or modify it under the
    terms of the GNU General Public License as published by the Free Software
    Foundation, either version 3 of the License, or (at your option) any later
    version.

    QTads is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
    details.

    You should have received a copy of the GNU General Public License along
    with QTads. If not, see <https://www.gnu.org/licenses/>.
*/

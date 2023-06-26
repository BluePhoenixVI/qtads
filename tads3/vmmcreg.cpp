#ifdef RCSID
static char RCSid[] =
"$Header: d:/cvsroot/tads/tads3/VMMCREG.CPP,v 1.2 1999/05/17 02:52:28 MJRoberts Exp $";
#endif

/* 
 *   Copyright (c) 1998, 2002 Michael J. Roberts.  All Rights Reserved.
 *   
 *   Please see the accompanying license file, LICENSE.TXT, for information
 *   on using and copying this software.  
 */
/*
Name
  vmmcreg.cpp - Metaclass Registry
Function
  The registry provides a static constant global table of the metaclasses
  that are linked in to this VM implementation.

  This file is dependent on the host application environment configuration.
  This particular file builds a table for the base set of T3 VM metaclasses.
  Some host environments may define an extended set of metaclasses; if
  you're building a host system with added metaclasses, do the following:

  1.  Make a copy of this file (DO NOT MODIFY THE ORIGINAL).

  2.  Remove this file (and/or files mechanically derived from this file,
      such as the compiled object file) from your makefile, and add your
      modified version of this file (and/or attendant derived files)
      instead.

  3.  At EACH line marked with a "!!!" comment below, add a #include
      for each of your added metaclass header files.  (You don't need
      to include the core metaclasses, such as list and string, since
      you should still include "vmmccore.h", which takes care of
      including the core files.)
Notes
  
Modified
  12/01/98 MJRoberts  - Creation
*/

#include "vmmcreg.h"

// !!! INCLUDE HOST-SPECIFIC METACLASS HEADERS HERE (FIRST OF TWO)

/* the global registry table */
#define VM_REGISTER_METACLASS(meta_class) {&meta_class::metaclass_reg_},

vm_meta_reg_t G_meta_reg_table[] =
{

// HACK: Form some reason, the MSVC compiler refuses to include a header file more than once.
// necessitating the macros that add the metaobjects be moved to here

VM_REGISTER_METACLASS(CVmObjAnonFn)
VM_REGISTER_METACLASS(CVmObjBigNum)
VM_REGISTER_METACLASS(CVmObjByteArray)
VM_REGISTER_METACLASS(CVmObjCollection)
VM_REGISTER_METACLASS(CVmObjCharSet)
VM_REGISTER_METACLASS(CVmObjDate)
VM_REGISTER_METACLASS(CVmObjDict)
VM_REGISTER_METACLASS(CVmDynamicFunc)
VM_REGISTER_METACLASS(CVmObjFileName)
VM_REGISTER_METACLASS(CVmObjFile)
VM_REGISTER_METACLASS(CVmObjFrameRef)
VM_REGISTER_METACLASS(CVmObjFrameDesc)
VM_REGISTER_METACLASS(CVmObjGramProd)
VM_REGISTER_METACLASS(CVmObjClass)
VM_REGISTER_METACLASS(CVmObjIter)
VM_REGISTER_METACLASS(CVmObjIterIdx)
VM_REGISTER_METACLASS(CVmObjLookupTable)
VM_REGISTER_METACLASS(CVmObjWeakRefLookupTable)
VM_REGISTER_METACLASS(CVmObjIterLookupTable)
VM_REGISTER_METACLASS(CVmObjList)
VM_REGISTER_METACLASS(CVmObject)
VM_REGISTER_METACLASS(CVmObjPattern)
VM_REGISTER_METACLASS(CVmObjString)
VM_REGISTER_METACLASS(CVmObjStringBuffer)
VM_REGISTER_METACLASS(CVmObjStrComp)
VM_REGISTER_METACLASS(CVmObjTemporaryFile)
VM_REGISTER_METACLASS(CVmObjTads)
VM_REGISTER_METACLASS(CVmObjIntClsMod)
VM_REGISTER_METACLASS(CVmObjTimeZone)
VM_REGISTER_METACLASS(CVmObjVector)

// !!! INCLUDE HOST-SPECIFIC METACLASS HEADERS HERE (SECOND OF TWO)

    /* 
     *   last entry in the table (this is only required because of the
     *   trailing comma in the registration macro) 
     */
    { 0 }
};

/* ------------------------------------------------------------------------ */
/*
 *   Register the metaclasses 
 */
void vm_register_metaclasses()
{
    uint i;
    vm_meta_reg_t *entry;
    
    /* 
     *   run through the metaclass table and tell each metaclass its
     *   registration table index 
     */
    for (i = 0, entry = G_meta_reg_table ; entry->meta != 0 ;
         ++i, ++entry)
    {
        /* 
         *   Call this entry's class method to set its registration index.
         *   This is static information that never changes throughout the
         *   program's execution - we simply establish the registration
         *   table index for each metaclass once upon initialization.  
         */
        (*entry->meta)->set_metaclass_reg_index(i);
    }
}

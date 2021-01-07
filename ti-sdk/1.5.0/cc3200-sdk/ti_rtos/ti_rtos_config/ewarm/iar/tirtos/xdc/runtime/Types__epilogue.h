/* 
 *  Copyright (c) 2008 Texas Instruments. All rights reserved. 
 *  This program and the accompanying materials are made available under the 
 *  terms of the Eclipse Public License v1.0 and Eclipse Distribution License
 *  v. 1.0 which accompanies this distribution. The Eclipse Public License is
 *  available at http://www.eclipse.org/legal/epl-v10.html and the Eclipse
 *  Distribution License is available at 
 *  http://www.eclipse.org/org/documents/edl-v10.php.
 *
 *  Contributors:
 *      Texas Instruments - initial implementation
 * */
/*
 *  ======== Types__epilogue.h ========
 *  Hand crafted macros for Types.xdc
 */

/*
 *  ======== xdc_runtime_Types_getEventId ========
 */
#define xdc_runtime_Types_getEventId(evt) (\
    (xdc_runtime_Types_RopeId)((evt) >> 16))

/*
 *  ======== xdc_runtime_Types_makeEvent ========
 */
#define xdc_runtime_Types_makeEvent(id, site) (\
    (xdc_runtime_Types_Event)(((Bits32) id) << 16 | ((site) & 0x0000ffff)))

/*
 *  ======== xdc_runtime_Types_getModuleId ========
 */
#define xdc_runtime_Types_getModuleId(evt) ((evt) & 0x0000ffff)
/*
 *  @(#) xdc.runtime; 2, 1, 0,0; 12-9-2015 17:36:49; /db/ztree/library/trees/xdc/xdc-B06/src/packages/
 */


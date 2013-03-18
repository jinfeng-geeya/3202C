/*********************************************************************
    Copyright (c) 2005 Embedded Internet Solutions, Inc
    All rights reserved. You are not allowed to copy or distribute
    the code without permission.
    This is the demo implenment of the SOUND Porting APIs needed by iPanel MiddleWare. 
    Maybe you should modify it accorrding to Platform.
    
    Note: the "int" in the file is 32bits
    
    $ver0.0.0.1 $author Zouxianyun 2005/04/28
*********************************************************************/

#include <stdio.h>
//#include <direct.h>
#include <udi/ipanel/ipanel_osfile.h>

/**********************************************************************************/
/*Description: open a OS file                                                     */
/*Input      : file name, and mode. the mode is only 'rb', other no supper        */
/*Output     : No                                                                 */
/*Return     : file description, -1 fail.                                         */
/**********************************************************************************/
int ipanel_porting_localfile_open(const char *filename, const char *mode)
{
	return 0;
}


/**********************************************************************************/
/*Description: Read data from the OS file                                         */
/*Input      : the buffer and max length to save data                             */
/*Output     : data                                                               */
/*Return     : real data length, -1 -- fail.                                      */
/**********************************************************************************/
int ipanel_porting_localfile_read(int fd, char* buffer, int nbytes)
{
	return 0;
}

/**********************************************************************************/
/*Description: Close the OS file                                                  */
/*Input      : the OS file description                                            */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail.                                          */
/**********************************************************************************/
int ipanel_porting_localfile_close(int fd)
{
	return 0;
}



/**********************************************************************************/
/*Description: open a OS directory                                                */
/*Input      : path name                                                          */
/*Output     : No                                                                 */
/*Return     : directory description, -1 fail.                                    */
/**********************************************************************************/
int ipanel_porting_localdir_open(const char *path)
{
	return 0;
}


/**********************************************************************************/
/*Description: Read data from the OS directory                                    */
/*Input      : address.                                                           */
/*Output     : directory EisDir data struct                                       */
/*Return     : 0 -- success, -1 -- fail.                                      */
/**********************************************************************************/
int ipanel_porting_localdir_read(int dd, EisDir **dir)
{
	return 0;
}


/**********************************************************************************/
/*Description: Close the OS directory                                             */
/*Input      : the OS directory description                                       */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 -- fail.                                          */
/**********************************************************************************/
int ipanel_porting_localdir_close(int dd)
{
	return 0;
}


/**********************************************************************************/
/*Description: Create a new OS directory                                          */
/*Input      : path name, directory name                                          */
/*Output     : No                                                                 */
/*Return     : 0 -- success, -1 fail.                                             */
/**********************************************************************************/
int ipanel_porting_localdir_mkdir(const char *path, const char *dir)
{
	return 0;
}


/******************************************************************************
 * $Id$
 *
 * Project:  MapServer
 * Purpose:  Python-specific enhancements to MapScript
 * Author:   Sean Gillies, sgillies@frii.com
 *
 ******************************************************************************
 *
 * Python-specific mapscript code has been moved into this 
 * SWIG interface file to improve the readibility of the main
 * interface file.  The main mapscript.i file includes this
 * file when SWIGPYTHON is defined (via 'swig -python ...').
 *
 *****************************************************************************/

/**************************************************************************
 * MapServer Errors and Python Exceptions
 **************************************************************************
 *
 * Translation of errors reported via the ms_error structure into
 * Python exceptions. Original code by Chris Chamberlin <cdc@aracnet.com>
 * has been updated by Sean Gillies <sgillies@frii.com> to use 
 * PyErr_SetString, %exception and $action for SWIG 1.3, do the most
 * obvious mapping of MapServer errors to Python exceptions and map all 
 * others to a new 'MapServerError' exception which can be caught like this:
 * 
 *   from mapscript import *
 *   empty_map = mapObj('')
 *   try:
 *       img = empty_map.draw()
 *   except MapServerError, msg:
 *       print "Caught MapServerError:", msg
 *
 *************************************************************************/

%{
PyObject *MSExc_MapServerError;
PyObject *MSExc_MapServerNotFoundError;
%}

%init %{

// Generic MapServer error
MSExc_MapServerError=PyErr_NewException("_mapscript.MapServerError",NULL,NULL);
if (MSExc_MapServerError != NULL)
    PyDict_SetItemString(d, "MapServerError", MSExc_MapServerError);

// MapServer query MS_NOTFOUND error
MSExc_MapServerNotFoundError = PyErr_NewException("_mapscript.MapServerNotFoundError", NULL, NULL);
if (MSExc_MapServerNotFoundError != NULL)
    PyDict_SetItemString(d, "MapServerNotFoundError", MSExc_MapServerNotFoundError);

%}

/* the new msGetErrorString is causing segfaults for some users so
 * we're going back to the home-made error stack output.  With the
 * Python MapScript the error stack should be rather short as 
 * MapServer errors are translated to Python exceptions and halt
 * execution. */
%{
    static void _raise_ms_exception(void) {
        char errbuf[256];
        char errmsg[2100]; // room for 8 * 256 byte messages + final notice
        int errcode;
        errorObj *ms_error;
        int error_depth;
        int max_error_depth = 8;
        
        strcpy(errmsg, "");
        ms_error = msGetErrorObj();
        errcode = ms_error->code;
        
        // Step through the error list, appending to the output error
        // message.
        error_depth = 0;
        while (ms_error && ms_error->code > 0) {
            if (error_depth == max_error_depth) {
                // Report that we've reached max error depth
                strcat(errmsg, "Reached maximum error depth (4)\n");
                break;
            }
            snprintf(errbuf, 255, "%s: %s %s\n", ms_error->routine,
                     msGetErrorCodeString(ms_error->code), ms_error->message);
            strcat(errmsg, errbuf);
            ms_error = ms_error->next;
            error_depth++;
        }

        // Map MapServer errors to Python exceptions, will define
        // custom Python exceptions soon.  The exception we raise
        // is based on the error at the head of the MapServer error
        // list.  All other errors appear in the error message.
        switch (errcode) {
            case MS_IOERR:
                PyErr_SetString(PyExc_IOError, errmsg);
                break;
            case MS_MEMERR:
                PyErr_SetString(PyExc_MemoryError, errmsg);
                break;
            case MS_TYPEERR:
                PyErr_SetString(PyExc_TypeError, errmsg);
                break;
            case MS_EOFERR:
                PyErr_SetString(PyExc_EOFError, errmsg);
                break;
            case MS_NOTFOUND:
                PyErr_SetString(MSExc_MapServerNotFoundError, errmsg);
                break;
            default:
                PyErr_SetString(MSExc_MapServerError, errmsg);
                break;
        }
    }
  
%}

%exception {
    $action {
        errorObj *ms_error = msGetErrorObj();
        // Sidestep the annoying MS_IOERR raised by msSearchDiskTree()
        // in maptree.c when a layer's shapetree index can't be found
        if ((ms_error->code != MS_NOERR) && (ms_error->code != -1)
        && !((ms_error->code == MS_IOERR) &&
             (strcmp(ms_error->routine, "msSearchDiskTree()") == 0)))
        {
            _raise_ms_exception();
            // Clear the error list
            msResetErrorList();
            return NULL;
        }
    }
}
         
%pythoncode %{
MapServerError = _mapscript.MapServerError
MapServerNotFoundError = _mapscript.MapServerNotFoundError
%}


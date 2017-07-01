#ifndef AUDIOTAG_H
#define AUDIOTAG_H

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;


#define MP3_TAG_ERR(m)  cerr << _argv0 << ": [error]  " << m << endl;
#define MP3_TAG_WARN(m)  cout << _argv0 << ": [warn]  " << m << endl;
#define MP3_TAG_NOTICE(m)  cout << _argv0 << ": " << m << endl;

#define MP3_TAG_WARN_VERBOSE(m)  if (_verbose)  { cout << _argv0 << ": [warn]  " << m << endl; }
#define MP3_TAG_NOTICE_VERBOSE(m)  if (_verbose)  { cout << _argv0 << ":  " << m << endl; }

#ifdef DEBUG
#define MP3_TAG_DEBUG(m)  cout << _argv0 << "; [DEBUG]  " << m << endl;
#else
#define MP3_TAG_DEBUG(m)  
#endif

#endif

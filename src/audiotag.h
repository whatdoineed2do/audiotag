#ifndef AUDIOTAG_H
#define AUDIOTAG_H

#include <iostream>
#include "../config.h"

namespace AudioTag
{
extern const char*  _argv0;
extern bool  _verbose;


#define AUDIOTAG_ERR(m)     std::cerr << AudioTag::_argv0 << ": [error]  " << m << std::endl;
#define AUDIOTAG_WARN(m)    std::cerr << AudioTag::_argv0 << ": [warn]  " << m << std::endl;
#define AUDIOTAG_NOTICE(m)  std::cout << AudioTag::_argv0 << ": " << m << std::endl;

#define AUDIOTAG_WARN_VERBOSE(m)    if (AudioTag::_verbose)  { std::cerr << AudioTag::_argv0 << ": [warn]  " << m << std::endl; }
#define AUDIOTAG_NOTICE_VERBOSE(m)  if (AudioTag::_verbose)  { std::cout << AudioTag::_argv0 << ":  " << m << std::endl; }

#ifdef DEBUG
#define AUDIOTAG_DEBUG(m)   std::cerr << AudioTag::_argv0 << "; [DEBUG]  " << m << std::endl;
#else
#define AUDIOTAG_DEBUG(m)  
#endif
};

#endif

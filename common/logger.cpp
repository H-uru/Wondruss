#include "logger.hpp"
#include <log4cplus/configurator.h>

// TODO: actually load a config file
log4cplus::Logger Wondruss::Logger::logger;

void Wondruss::Logger::init(const char* channel)
{
  log4cplus::PropertyConfigurator::doConfigure("log.conf");
  logger = log4cplus::Logger::getInstance(channel);
}

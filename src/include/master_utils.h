/**
 * master.h
 * This file contains the main operatrions like shutdown and command line parsing
 */

#pragma once

extern void shutdown(int exit_code);
extern int parse_command_line(int argc, char *argv[]);
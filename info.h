/**
* \file info.h
 * Utilities to show program info.
 */
#pragma once

/**
 * Helper to print the program's context info, e.g.:
 * \code
 *	GL version: 4.1 Metal - 90.5
 *	Hardware has DXT1/BC1 support: yes
 *	Hardware has DXT5/BC3 support: yes
 *	Hardware has RGTC/BC4 support: no
 * \endcode
 */
void showInfo();

/**
 * Helper to print the program's command line options.
 *
 * \param[in] path optional full path of the application (from \c argv)
 */
void showUsage(const char* path);

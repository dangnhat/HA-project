/**
 * @file shell_cmds_fatfs.h
 * @author  Pham Huu Dang Nhat  <phamhuudangnhat@gmail.com>, HLib MBoard team.
 * @version 1.0
 * @date 27-Oct-2014
 * @brief Header files for FAT-FS related shell commands.
 */

#ifndef SHELL_CMDS_FATFS_H_
#define SHELL_CMDS_FATFS_H_

#include "ff.h"

/**
 * @brief   Mount physical drive to default logical drive (0:/).
 *
 * @details Usage: mount.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void mount(int argc, char** argv);

/**
 * @brief   Un-mount default logical drive (0:/).
 *
 * @details Usage: umount.
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void umount(int argc, char** argv);

/**
 * @brief   List files and folders in current directory of from given path
 *
 * @details Usage: ls [path]
 *                 `ls` do the listing for the current directory
 *                 `ls "0:/path"` do the listing for path folder in drive 0
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void ls(int argc, char** argv);

/**
 * @brief   Concatenate files and print on the standard output
 *
 * @details Usage: cat [file]
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void cat(int argc, char** argv);

/**
 * @brief   Change timestamp of an existing file.
 *
 * @details Usage: touch [file]
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void touch(int argc, char** argv);

/**
 * @brief   Append a string to file.
 *
 * @details Usage: echo [string] [file]
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void echo(int argc, char** argv);

/**
 * @brief   Remove a file or folder.
 *
 * @details Usage: rm [file/folder]
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void rm(int argc, char** argv);

/**
 * @brief   Move a file or a folder.
 *
 * @details Usage: mv [old_path] [new_path]
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void mv(int argc, char** argv);

/**
 * @brief   Create a new folder.
 *
 * @details Usage: mkdir [new_folder]
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void mkdir(int argc, char** argv);

/**
 * @brief   Change current directory.
 *
 * @details Usage: cd [path]
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void cd(int argc, char** argv);

/**
 * @brief   Print current directory.
 *
 * @details Usage: pwd
 *
 * @param[in] argc  Argument count
 * @param[in] argv  Arguments
 */
void pwd(int argc, char** argv);

/**
 * @brief   Print error of FAT file system module.
 *
 * @param[in] res  result of FAT operation.
 */
void print_ferr(FRESULT res);

#endif /* SHELL_CMDS_FATFS_H_ */

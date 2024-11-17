/**
 * cache_errno.h
 * This file contains enum with error codes and global variable to communicate
 * these codes between different functions and cache modules (aka errno).
 */
#pragma once
// TO_DO Add enum support for GUC and move it
enum cache_err_num
{
    NO_FREE_SPACE,
    TTL_ELAPSED,
    OK
};
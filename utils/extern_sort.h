#ifndef _DATA_JASONYJIANG_COMMON_PROJ_C___TEST_EXTERN_SORT_H
#define _DATA_JASONYJIANG_COMMON_PROJ_C___TEST_EXTERN_SORT_H
#pragma once
#include <stdint.h>
#include <sys/time.h>
#include <errno.h>
#include <string.h>
#include <assert.h>

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <iostream>
#include <vector>
#include <sstream>

#define RANK_SYMBOL >

typedef std::pair<double, std::string> SSortItem;

class CBaseExternSort
{
public:
    CBaseExternSort();
    ~CBaseExternSort();
    bool Init(const std::string& tmp_path, uint32_t sort_num = 1000000);
    bool Sort();

protected:
    virtual bool GetItem(double *rank, std::string* item) = 0;
    virtual bool SetItem(double rank, const std::string& item) = 0;

private:
    bool WriteFile();
    bool ReadFile(FILE *fp, double *rank, std::string *item);
    bool MemorySort();
    bool MergeSort();

private:
    char *m_buffer;
    uint32_t m_sort_num;
    uint32_t m_buffer_len;
    std::string m_path;
    std::string m_version;
    std::vector<std::string> m_sort_file;
    std::vector<SSortItem> m_sort_item;
};

class CTestSort : public CBaseExternSort {
protected :
    virtual bool GetItem(double *rank, std::string* item);
    virtual bool SetItem(double rank, const std::string& item);
private:
};


#endif // _DATA_JASONYJIANG_COMMON_PROJ_C___TEST_EXTERN_SORT_H

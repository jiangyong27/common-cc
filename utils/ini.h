// Copyright (c) 2013, Tencent Inc.
// All rights reserved.
//
// Author:jason  <jasonyjiang@tencent.com>
// Created: 2015-01-05
//

#ifndef _HOME_UBUNTU_TEST_INI_H
#define _HOME_UBUNTU_TEST_INI_H
#pragma once
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <fstream>
#include <map>

class CIniFile
{
public:
    CIniFile();
    ~CIniFile();
    bool OpenIniFile(const std::string& path);

    void GetFiledValue(const std::string& field, const std::string& key, const std::string& def, std::string* value);
    void GetFiledValue(const std::string& field, const std::string& key, uint32_t def, uint32_t *value);
    void GetFiledValue(const std::string& field, const std::string& key, int def, int *value);
    void GetFiledValue(const std::string& field, const std::string& key, bool def, bool *value);
    void GetFiledValue(const std::string& field, const std::string& key, double def, double *value);
private:
    bool GetFiledValue(const std::string& field, const std::string& key, std::string* value);
    std::string m_file;
    std::map<std::string, std::string> m_kv;
};

#endif // _HOME_UBUNTU_TEST_INI_H

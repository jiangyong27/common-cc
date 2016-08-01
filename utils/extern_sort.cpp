#include "extern_sort.h"

CBaseExternSort::CBaseExternSort()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    std::stringstream ss;
    ss << tv.tv_sec << "_" << tv.tv_usec;
    m_version = ss.str();

    m_path = ".";
    m_sort_file.clear();
    m_buffer_len = 0;
    m_buffer = NULL;
}

CBaseExternSort::~CBaseExternSort()
{

}

bool CBaseExternSort::Init(const std::string& tmp_path, uint32_t sort_num)
{
    m_path = tmp_path;
    m_sort_num = sort_num;
    m_sort_file.clear();
    m_sort_item.clear();
    m_buffer_len = 0;

}

bool CBaseExternSort::Sort()
{
    if (!MemorySort()) {
        return false;
    }
    if (!MergeSort()) {
        return false;
    }
    return true;
}

bool SortFunction(std::pair<double, std::string> s1, std::pair<double, std::string> s2)
{
    return s1.first RANK_SYMBOL s2.first;
    //return s1.first < s2.first;
}

bool CBaseExternSort::ReadFile(FILE *fp, double *rank, std::string *item)
{
    if (!fp) {
        return false;
    }

    if (1 != fread(rank, sizeof(double), 1, fp)) {
        return false;
    }

    uint32_t len;
    if (1 != fread(&len, sizeof(uint32_t), 1, fp)) {
        printf("[1]fread  failed![error=%s]\n", strerror(errno));
        return false;
    }

    if (1 != fread(m_buffer, len, 1, fp)) {
        printf("[2]fread  failed![error=%s]\n", strerror(errno));
    }

    item->assign(m_buffer, len);
    return true;
}

bool CBaseExternSort::WriteFile()
{
    char tmp_buf[512];
    //snprintf(tmp_buf, sizeof(tmp_buf), "%s/extern_sort_%s_%d.dat", m_path.c_str(), m_version.c_str(), m_sort_file.size());
    snprintf(tmp_buf, sizeof(tmp_buf), "%s/extern_sort_%d.dat", m_path.c_str(), m_sort_file.size());
    m_sort_file.push_back(tmp_buf);

    FILE *fp = fopen(tmp_buf, "w");
    if (fp == NULL) {
        printf("fopen file[%s] failed![error=%s]\n", tmp_buf, strerror(errno));
        return false;
    }

    for (uint32_t i = 0;i < m_sort_item.size(); ++i) {
        uint32_t st =  m_sort_item[i].second.size();
        if (st > m_buffer_len) m_buffer_len = st + 1;
        if (1 != fwrite(&m_sort_item[i].first, sizeof(double), 1, fp)) {
            printf("[0]fwrite file[%s] failed![error=%s]\n", tmp_buf, strerror(errno));
            fclose(fp);
            return false;
        }

        if (1 != fwrite(&st, sizeof(uint32_t), 1, fp)) {
            printf("[1]fwrite file[%s] failed![error=%s]\n", tmp_buf, strerror(errno));
            fclose(fp);
            return false;
        }

        if (1 != fwrite(m_sort_item[i].second.c_str(), st, 1, fp)) {
            printf("[2]fwrite file[%s] len[%d] failed![error=%s]\n", tmp_buf, m_sort_item[i].second.size(), strerror(errno));
            fclose(fp);
            return false;
        }
    }
    fclose(fp);
    return true;
}

bool CBaseExternSort::MemorySort()
{
    std::string item;
    double rank;
    uint32_t count = 0;

    while (GetItem(&rank, &item)) {
        m_sort_item.push_back(std::make_pair<double, std::string>(rank, item));
        count++;
        if (count % m_sort_num == 0) {
            std::sort(m_sort_item.begin(), m_sort_item.end(), SortFunction);
            if (!WriteFile()) return false;
            m_sort_item.clear();
        }
    }

    if (m_sort_item.size() != 0) {
        std::sort(m_sort_item.begin(), m_sort_item.end(), SortFunction);
        if (!WriteFile()) return false;
        m_sort_item.clear();
    }
    return true;
}

bool CBaseExternSort::MergeSort()
{
    std::vector<FILE *> fp_list;
    for (uint32_t i = 0; i < m_sort_file.size(); i++) {
        FILE* fp = fopen(m_sort_file[i].c_str(), "rb");
        assert(fp != NULL);
        fp_list.push_back(fp);
    }

    if (m_buffer != NULL) delete [] m_buffer;
    m_buffer = new char[m_buffer_len];

    double rank;
    std::string item;

    std::vector<std::string> vect_item;
    std::vector<double> vect_rank;
    vect_rank.resize(fp_list.size());
    vect_item.resize(fp_list.size());

    for (uint32_t i = 0; i < fp_list.size(); i++) {
        if (!ReadFile(fp_list[i], &rank, &item)) {
            fclose(fp_list[i]);
            fp_list[i] = NULL;
            continue;
        }
        vect_rank[i] = rank;
        vect_item[i].assign(item);
    }

    // 一边读取一边merge写入文件
    double max_rank = vect_rank[0];
    uint32_t max_index = 0;
    bool quit;
    while (1) {
        for (uint32_t i = 0; i < vect_rank.size(); ++i) {
            if (fp_list[i] == NULL) continue;
            if (vect_rank[i] RANK_SYMBOL max_rank) {
                max_rank = vect_rank[i];
                max_index = i;
            }
        }

        SetItem(vect_rank[max_index], vect_item[max_index]);
        if (!ReadFile(fp_list[max_index], &rank, &item)) {
            fclose(fp_list[max_index]);
            fp_list[max_index] = NULL;
            quit = true;
            for (uint32_t j = 0;j < fp_list.size(); ++j) {
                if (fp_list[j] != NULL) {
                    max_index = j;
                    max_rank = vect_rank[j];
                    quit = false;
                    break;
                }
            }
            if (quit) break;
        } else {
            max_rank = rank;
            vect_rank[max_index] = rank;
            vect_item[max_index].assign(item);
        }

    }


    // 删除临时文件
    for (uint32_t i = 0; i < m_sort_file.size(); i++) {
        remove(m_sort_file[i].c_str());
    }

    delete [] m_buffer;
    m_buffer = NULL;
    return true;
}


bool CTestSort::GetItem(double *rank, std::string* item)
{
    static uint32_t count = 0;
    count++;
    if (count % 10000000 == 0) {
        return false;
    }
    if (count == 0) {
        srand(time(NULL));
    }

    std::stringstream ss;
    *rank = (rand() % 10000);

    ss << *rank;
    item->assign(ss.str());
    return true;
}

bool CTestSort::SetItem(double rank, const std::string& item)
{
    printf("%f : %s\n", rank, item.c_str());
    return true;
}

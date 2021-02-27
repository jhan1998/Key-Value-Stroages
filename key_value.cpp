#include <bits/stdc++.h>
#include <sys/stat.h>

#define MAX_SIZE 50000000
#define MAX_REF 1111111
unsigned long int num_ref[9];

using namespace std;

inline int classify(unsigned long long x)
{
    stringstream tmp_s;
    tmp_s << x;
    return tmp_s.str()[0] - '0' - 1;
}

inline uint64_t bitshift(string const &value)
{
    uint64_t result = 0;

    char const *p = value.c_str();
    char const *q = p + value.size();
    while (p < q)
    {
        result = (result << 1) + (result << 3) + *(p++) - '0';
    }
    return result;
}

void pop_last(unordered_map<unsigned long long int, pair<string, list<pair<unsigned long long int, bool>>::iterator>> &store_buffer,
              list<pair<unsigned long long int, bool>> &store_list,
              unordered_map<unsigned long long int, unsigned long> &disk_ref,
              unsigned long offset, FILE **disk,
              unsigned long long int key)
{
    auto last = --store_list.end();
    if (last->second)
    {
        string *str = new string();
        *str = store_buffer.find(last->first)->second.first;
        auto itr = disk_ref.find(last->first);
        int loc = classify(last->first);
        // cout << last->first << endl;
        // cout << "PUT IN " << loc << endl;
        if (itr != disk_ref.end())
        {
            fseek(disk[loc], itr->second + sizeof(unsigned long long), SEEK_SET);
            fwrite(store_buffer.find(last->first)->second.first.c_str(), sizeof(char), 129, disk[loc]);
            fseek(disk[loc], 0, SEEK_END);
        }
        else if (itr == disk_ref.end())
        {
            bool is_find = false;
            unsigned long int counter = 0;
            fseek(disk[loc], 0, SEEK_SET);
            unsigned long long search;
            while (fread(&search, sizeof(unsigned long long), 1, disk[loc]))
            {
                if (search == last->first)
                {
                    is_find = true;
                    break;
                }
                counter++;
                fseek(disk[loc], 129, SEEK_CUR);
            }
            if (!is_find)
            {
                fseek(disk[loc], 0, SEEK_END);
                fwrite(&last->first, sizeof(unsigned long long), 1, disk[loc]);
                fwrite(store_buffer.find(last->first)->second.first.c_str(), sizeof(char), 129, disk[loc]);
                if (disk_ref.size() == MAX_REF)
                {
                    auto it = disk_ref.begin();
                    srand(time(NULL));
                    advance(it, rand() % disk_ref.size());
                    while (it->first == key)
                    {
                        it = disk_ref.begin();
                        advance(it, rand() % disk_ref.size());
                    }
                    disk_ref.erase(it);
                }
                disk_ref[last->first] = num_ref[loc] * offset;
                num_ref[loc] += 1;
            }
            else
            {
                fseek(disk[loc], counter * offset + sizeof(unsigned long long), SEEK_SET);
                fwrite(store_buffer.find(last->first)->second.first.c_str(), sizeof(char), 129, disk[loc]);
                if (disk_ref.size() == MAX_REF)
                {
                    auto it = disk_ref.begin();
                    srand(time(NULL));
                    advance(it, rand() % disk_ref.size());
                    while (it->first == key)
                    {
                        it = disk_ref.begin();
                        advance(it, rand() % disk_ref.size());
                    }
                    disk_ref.erase(it);
                }
                disk_ref[last->first] = counter * offset;
                fseek(disk[loc], 0, SEEK_END);
            }
        }
    }
    store_list.erase(last);
    store_buffer.erase(store_buffer.find(last->first));
}

int main(int argc, char *argv[])
{
    FILE *in = fopen(argv[1], "r");
    string s(argv[1]);
    string name = ".output";
    name = s[s.length() - 7] + name;
    // cout << name << endl;
    FILE *out = fopen(name.c_str(), "w");
    char buffer[1000];
    int get_flag = 0;
    mkdir("./storage", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
    list<pair<unsigned long long int, bool>> store_list;
    unordered_map<unsigned long long int, pair<string, list<pair<unsigned long long int, bool>>::iterator>> store_buffer;
    unordered_map<unsigned long long int, unsigned long> disk_ref;
    auto offset = sizeof(unsigned long long) + 129;
    FILE *disk[9];
    for (int i = 0; i < 9; i++)
    {
        stringstream ss;
        ss << "storage/disk_" << i << ".tmp";
        disk[i] = fopen(ss.str().c_str(), "r+");
        if (!disk[i])
        {
            disk[i] = fopen(ss.str().c_str(), "w+");
        }
    }
    FILE *ref_in = fopen("storage/disk_ref.tmp", "a+");
    if (!ref_in)
    {
        for (int i = 0; i < 9; i++)
            num_ref[i] = 0;
        fclose(ref_in);
    }
    else
    {
        fseek(ref_in, 0, SEEK_SET);
        for (int i = 0; i < 9; i++)
            fread(&num_ref[i], sizeof(unsigned long int), 1, ref_in);
        unsigned long long int key;
        unsigned long int off;
        while (fread(&key, sizeof(unsigned long long int), 1, ref_in))
        {
            fread(&off, sizeof(unsigned long int), 1, ref_in);
            disk_ref[key] = off;
        }
        fclose(ref_in);
    }
    FILE *list_in = fopen("storage/list.tmp", "a+");
    if (!list_in)
    {
        fclose(list_in);
    }
    else
    {
        fseek(list_in, 0, SEEK_SET);
        unsigned long long int key;
        while (fread(&key, sizeof(unsigned long long int), 1, list_in))
        {
            auto itr = disk_ref.find(key);
            int loc = classify(key);
            if (itr != disk_ref.end())
            {
                fseek(disk[loc], itr->second + sizeof(unsigned long long), SEEK_SET);
                char *str = (char *)malloc(sizeof(char) * 129);
                fread(str, sizeof(char), 129, disk[loc]);
                string value = str;
                store_list.push_front(pair<unsigned long long int, bool>(key, false));
                store_buffer[key] = make_pair(value, store_list.begin());
                fseek(disk[loc], 0, SEEK_END);
            }
            else if (itr == disk_ref.end())
            {
                unsigned long long tmp;
                fseek(disk[loc], 0, SEEK_SET);
                while (fread(&tmp, sizeof(unsigned long long), 1, disk[loc]))
                {
                    if (tmp == key)
                    {
                        char *str = (char *)malloc(sizeof(char) * 129);
                        fread(str, sizeof(char), 129, disk[loc]);
                        string value = str;
                        store_list.push_front(pair<unsigned long long int, bool>(key, false));
                        store_buffer[key] = make_pair(value, store_list.begin());
                        fseek(disk[loc], 0, SEEK_END);
                        break;
                    }
                    fseek(disk[loc], 129, SEEK_CUR);
                }
            }
        }
        fclose(list_in);
    }
    while (fgets(buffer, 1000, in))
    {
        //buffer[strlen(buffer) - 1] = '\0';
        char *token;
        const char *delim = " \n";
        token = strtok(buffer, delim);
        string key, key2, value;
        if (!strcmp("PUT", token))
        {
            key = strtok(NULL, delim);
            value = strtok(NULL, delim);
            unsigned long long int_key = bitshift(key);
            // out of the buffer
            if (store_buffer.find(int_key) == store_buffer.end())
            {
                if (store_list.size() == MAX_SIZE) // bufer is full
                {
                    pop_last(store_buffer, store_list, disk_ref, offset, disk, int_key);
                }
                store_list.push_front(make_pair(int_key, true));
                store_buffer[int_key] = make_pair(value, store_list.begin());
            }
            // In the buffer
            else
            {
                store_list.erase(store_buffer[int_key].second);
                store_list.push_front(make_pair(int_key, true));
                store_buffer[int_key] = make_pair(value, store_list.begin());
            }
        }
        else if (!strcmp("GET", token))
        {
            get_flag = 1;
            key = strtok(NULL, delim);
            unsigned long long int int_key = bitshift(key);
            auto itr = store_buffer.find(int_key);
            if (itr != store_buffer.end())
            {
                fputs((itr->second.first + "\n").c_str(), out);
                store_list.erase(store_buffer[int_key].second);
                store_list.push_front(make_pair(int_key, itr->second.second->second));
            }
            else
            {
                if (store_list.size() != MAX_SIZE)
                    fputs("EMPTY\n", out);
                else // pop
                {
                    auto itr = disk_ref.find(int_key);
                    int loc = classify(int_key);
                    if (itr != disk_ref.end())
                    {
                        fseek(disk[loc], itr->second + sizeof(unsigned long long), SEEK_SET);
                        char *str = (char *)malloc(sizeof(char) * 129);
                        fread(str, sizeof(char), 129, disk[loc]);
                        string value = str;
                        fputs((value + "\n").c_str(), out);
                        pop_last(store_buffer, store_list, disk_ref, offset, disk, int_key);
                        store_list.push_front(pair<unsigned long long int, bool>(int_key, false));
                        store_buffer[int_key] = make_pair(value, store_list.begin());
                        fseek(disk[loc], 0, SEEK_END);
                    }
                    else if (itr == disk_ref.end())
                    {
                        bool is_find = false;
                        unsigned long long tmp;
                        fseek(disk[loc], 0, SEEK_SET);
                        while (fread(&tmp, sizeof(unsigned long long), 1, disk[loc]))
                        {
                            if (tmp == int_key)
                            {
                                is_find = true;
                                char *str = (char *)malloc(sizeof(char) * 129);
                                fread(str, sizeof(char), 129, disk[loc]);
                                string value = str;
                                fputs((value + "\n").c_str(), out);
                                pop_last(store_buffer, store_list, disk_ref, offset, disk, int_key);
                                store_list.push_front(pair<unsigned long long int, bool>(tmp, false));
                                store_buffer[int_key] = make_pair(value, store_list.begin());
                                fseek(disk[loc], 0, SEEK_END);
                                break;
                            }
                            fseek(disk[loc], 129, SEEK_CUR);
                        }
                        if (!is_find)
                        {
                            fputs("EMPTY\n", out);
                        }
                    }
                }
            }
        }
        else if (!strcmp("SCAN", token))
        {
            get_flag = 1;
            key = strtok(NULL, delim);
            key2 = strtok(NULL, delim);
            unsigned long long start = bitshift(key);
            unsigned long long end = bitshift(key2);
            for (unsigned long long i = start; i <= end; i++)
            {
                // cout << "GET " << i << endl;
                auto itr = store_buffer.find(i);
                if (itr == store_buffer.end())
                {
                    if (store_list.size() != MAX_SIZE)
                        fputs("EMPTY\n", out);
                    else // pop
                    {
                        auto itr = disk_ref.find(i);
                        int loc = classify(i);
                        if (itr != disk_ref.end())
                        {
                            fseek(disk[loc], itr->second + sizeof(unsigned long long), SEEK_SET);
                            char *str = (char *)malloc(sizeof(char) * 129);
                            fread(str, sizeof(char), 129, disk[loc]);
                            string value = str;
                            fputs((value + "\n").c_str(), out);
                            pop_last(store_buffer, store_list, disk_ref, offset, disk, i);
                            store_list.push_front(pair<unsigned long long int, bool>(i, false));
                            store_buffer[i] = make_pair(value, store_list.begin());
                            fseek(disk[loc], 0, SEEK_END);
                        }
                        else if (itr == disk_ref.end())
                        {
                            bool is_find = false;
                            unsigned long long tmp;
                            fseek(disk[loc], 0, SEEK_SET);
                            while (fread(&tmp, sizeof(unsigned long long), 1, disk[loc]))
                            {
                                if (tmp == i)
                                {
                                    is_find = true;
                                    char *str = (char *)malloc(sizeof(char) * 129);
                                    fread(str, sizeof(char), 129, disk[loc]);
                                    string value = str;
                                    fputs((value + "\n").c_str(), out);
                                    pop_last(store_buffer, store_list, disk_ref, offset, disk, i);
                                    store_list.push_front(pair<unsigned long long int, bool>(tmp, false));
                                    store_buffer[i] = make_pair(value, store_list.begin());
                                    fseek(disk[loc], 0, SEEK_END);
                                    break;
                                }
                                fseek(disk[loc], 129, SEEK_CUR);
                            }
                            if (!is_find)
                            {
                                fputs("EMPTY\n", out);
                            }
                        }
                    }
                }
                else
                {
                    fputs((itr->second.first + "\n").c_str(), out);
                    store_list.erase(store_buffer[i].second);
                    store_list.push_front(make_pair(i, itr->second.second->second));
                }
            }
        }
        // cout << "------------\n";
        // for (auto itr = store_list.begin(); itr != store_list.end(); itr++)
        //     cout << "IN THE MEMORY : " << itr->first << endl;
        // cout << "------------\n";
        // for (int i = 0; i < 9; i++)
        // {
        //     unsigned long long a;
        //     fseek(disk[i], 0, SEEK_SET);
        //     while (fread(&a, sizeof(unsigned long long), 1, disk[i]))
        //     {
        //         char *str = (char *)malloc(sizeof(char) * 129);
        //         fread(str, sizeof(char), 129, disk[i]);
        //         cout << "IN THE DISK key : " << a << " value : " << str << endl;
        //     }
        // }
        // cout << "------------\n";
    }
    FILE *list_s = fopen("storage/list.tmp", "w");
    while (store_list.size() != 0)
    {
        fwrite(&store_list.back().first, sizeof(unsigned long long), 1, list_s);
        pop_last(store_buffer, store_list, disk_ref, offset, disk, 0);
    }
    fclose(in);
    fclose(out);
    fclose(list_s);
    if (get_flag == 0)
    {
        remove(name.c_str());
    }
    for (int i = 0; i < 9; i++)
        fclose(disk[i]);

    FILE *ref_s = fopen("storage/disk_ref.tmp", "w");
    for (int i = 0; i < 9; i++)
    {
        fwrite(&num_ref[i], sizeof(unsigned long), 1, ref_s);
    }
    for (auto itr = disk_ref.begin(); itr != disk_ref.end(); itr++)
    {
        fwrite(&itr->first, sizeof(unsigned long long), 1, ref_s);
        fwrite(&itr->second, sizeof(unsigned long), 1, ref_s);
    }
    fclose(ref_s);

    return 0;
}

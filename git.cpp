#include <iostream>
#include <sys/types.h>
#include <fstream>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <bits/stdc++.h>

#include "sha1.hpp"

#define REDTEXT "\x1B[31m"
#define GREENTEXT "\x1B[32m"
#define YELLOWTEXT "\x1B[33m"
#define RESET "\033[0m"

#define LOG_FILE "./.git/log.txt"
#define TrackerLog "./trackerlog.txt"

#define IN ifstream::in
#define OUT ios_base::out
#define APPEND ios_base::app

#define STAT struct stat
#define DIRENT struct dirent

using namespace std;

/**** Global Variables ****/
set<string> current_directory;
vector<dirent *> dirfiles;

/*** Function Prototypes ****/
void trackerlogs(string);
void log_update(string);
void error_display(string);
bool dir_checker(const string &);
bool getFileSize(string);
int cnt_dir(string);
vector<string> splitter(string, char);
vector<string> file_reader(string);
unordered_set<string> fileset_reader(string);
unordered_map<string, string> filemap_reader(string);
bool mymkdir(string);
void del_data(string);
void del_file(string);
void cp_file(string, string);
void cp_dir(string, string);
void myls(const char *, vector<string> &);
void write_to_file(vector<string>, string);
void versionFolder_add(vector<string> &, string);
void modify_status(vector<string>);
void init();
void add(string);
void commitUtil(string, vector<string> &, unordered_map<string, string> &);
string mycustom_msg(string);
void modify_addFiles(vector<string> &, unordered_map<string, string>);
void commit(string);
void printUntracked(vector<string>);
void printTbc(vector<string>);
void printModified(vector<string>);
void status();
bool mycustomcomparator(dirent *, dirent *);
void explore_dir(string, vector<string> &);
int myrmdir(const char *);
void rmfolder(vector<string>);
void cpfile2(string, string, char *);
void cpdir2(const char *, const char *, char *);
void cp_ver(const char *, const char *);
void rollback();
void log();
bool init_first();
void menu(vector<string>);

/**** Functions ****/
void trackerlogs(string text)
{
    ofstream log_file(TrackerLog, OUT | APPEND);
    log_file << text;
    log_file.close();
}

void log_update(string text)
{
    ofstream log_file(LOG_FILE, OUT | APPEND);
    log_file << text;
    log_file.close();
}

void error_display(string str)
{
    perror(str.c_str());
    exit(0);
}

bool dir_checker(const string &path)
{
    STAT info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }

    if (info.st_mode & S_IFDIR)
    {
        return true;
    }
    return false;
}

bool getFileSize(string path)
{
    STAT file_stat;

    if (stat(path.c_str(), &file_stat) != -1)
    {
        if (!S_ISREG(file_stat.st_mode))
        {
            return false;
        }
        return file_stat.st_size;
    }
    return false;
}

int cnt_dir(string path)
{

    DIR *dir;
    DIRENT *dp;
    int count = 0;

    dir = opendir(path.c_str());

    if (dir != NULL)
    {
        while ((dp = readdir(dir)) != NULL)
        {
            if ((string)dp->d_name != "." && (string)dp->d_name != "..")
            {
                if (dp->d_type == DT_DIR)
                {
                    count++;
                }
            }
        }
        closedir(dir);
        return count;
    }

    return 0;
}

vector<string> splitter(string text, char delimiter)
{
    vector<string> result;
    string temp = "";
    for (size_t i = 0; i < text.size(); i++)
    {
        if (text[i] == delimiter)
        {
            result.push_back(temp);
            temp = "";
        }
        else
        {
            temp += text[i];
        }
    }
    result.push_back(temp);

    return result;
}

vector<string> file_reader(string file_name)
{

    string next_line;
    vector<string> result;

    ifstream file_stream(file_name, IN);

    while (getline(file_stream, next_line))
    {
        result.push_back(next_line);
    }

    file_stream.close();
    return result;
}

unordered_set<string> fileset_reader(string file_name)
{
    vector<string> res1 = file_reader(file_name);

    unordered_set<string> res2;
    for (auto s : res1)
    {
        res2.insert(s);
    }

    return res2;
}

unordered_map<string, string> filemap_reader(string file_name)
{
    unordered_map<string, string> result;
    string get_line;
    int fl;

    ifstream ifs;
    ifs.open(file_name);

    while (!ifs.eof())
    {
        if (getline(ifs, get_line))
        {
            vector<string> words = splitter(get_line, ' ');
            fl = 1;
            result[words[0]] = words[1];
        }
    }

    ifs.close();
    return result;
}

bool mymkdir(string fpath)
{

    const char *cc_path = fpath.c_str();

    mode_t fmode = 0777;

    short status = mkdir(cc_path, fmode);
    if (status == 0)
    {
        return true;
    }

    if (ENOENT == errno)
    {
        long psn = fpath.find_last_of('/');

        if (fpath.find_last_of('/') == string::npos)
        {
            return false;
        }

        if (!mymkdir(fpath.substr(0, psn)))
        {
            return false;
        }

        return mkdir(cc_path, fmode) == 0;
    }
    else if (EEXIST == errno)
    {
        return dir_checker(fpath);
    }

    return false;
}

void del_data(string file_name)
{
    ofstream log_file(file_name, ios_base::out);
    log_file.close();
}

void del_file(string Destination)
{
    if (unlink(Destination.c_str()) != 0)
    {
        error_display("Error deleting file\n");
        trackerlogs("Error deleting file inside function\n");
    }
}

void cp_file(string src_path, string dest_path)
{
    char buff[BUFSIZ];
    struct stat st;
    int size;
    FILE *dest_file;
    FILE *src_file;

    dest_file = fopen(&dest_path[0], "w");
    src_file = fopen(&src_path[0], "r");

    size = fread(buff, 1, BUFSIZ, src_file);
    while (1)
    {
        fwrite(buff, 1, size, dest_file);
        size = fread(buff, 1, BUFSIZ, src_file);
        if (size <= 0)
            break;
    }

    stat(src_path.c_str(), &st);
    chmod(dest_path.c_str(), st.st_mode);

    fclose(src_file);
    fclose(dest_file);
}

void cp_dir(string s1, string s2)
{
    DIR *d;
    struct dirent *file;
    struct stat buf;
    d = opendir(s1.c_str());
    while ((file = readdir(d)) != NULL)
    {
        stat((s1 + "/" + file->d_name).c_str(), &buf);
        if (S_ISREG(buf.st_mode))
        {
            cp_file(s1 + "/" + string(file->d_name), s2 + "/" + string(file->d_name));
        }
        else
        {
            if (string(file->d_name) != "." && string(file->d_name) != "..")
            {
                mkdir((s2 + "/" + string(file->d_name)).c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
                cp_dir(s1 + "/" + string(file->d_name), s2 + "/" + string(file->d_name));
            }
        }
    }
    chdir("..");
    closedir(d);
    return;
}

void myls(const char *dirname, vector<string> &files)
{
    DIR *dir;
    dir = opendir(dirname);
    if (dir == NULL)
    {
        return;
    }
    DIRENT *entity = readdir(dir);
    while (entity != NULL)
    {
        string dir_content = string(entity->d_name);
        if (dir_content != "." && dir_content != ".." && dir_content != ".git")
        {
            if (string(dirname) != ".")
            {
                string path = string(dirname) + "/" + dir_content;

                path = path.substr(2);
                if (!dir_checker(path))
                    files.push_back(path);
            }
            else if (entity->d_type == DT_REG)
            {
                files.push_back(entity->d_name);
            }
            else if (entity->d_type == DT_DIR)
            {
                string path = string(dirname) + "/" + string(entity->d_name);
                myls(path.c_str(), files);
            }
        }
        entity = readdir(dir);
    }
    closedir(dir);
}

void write_to_file(vector<string> files, string file) {
	ofstream out(file);
	for(const auto &e: files)
	{
		out << e << "\n";
	}
}

void versionFolder_add(vector<string> &files, string dest)
{

    string src;
    int i = 0;
    int pos;
    string temp = dest + "/";

    for (; i < files.size(); i++)
    {
        src = files[i];
        pos = src.find_last_of('/');

        if (pos == string::npos)
        {
            mymkdir(temp);
        }
        else
        {
            mymkdir(temp + src.substr(0, pos));
        }

        cp_file("./" + src, temp + src);
    }
}

void modify_status(vector<string> files)
{
    string hash;
    string dest;
    string temp;
    vector<string> file_hash;
    int i = 0;

    while (i < files.size())
    {
        hash = SHA1::from_file(files[i]);
        temp = files[i] + " " + hash;
        file_hash.push_back(temp);

        i++;
    }

    dest = "./.git/status.txt";

    write_to_file(file_hash, dest);
}


void init()
{
    char path[PATH_MAX], new_path;
    getcwd(path, PATH_MAX);
    string HomeDir;
    HomeDir = (string)path + "/.git";
    int check = mkdir(HomeDir.c_str(), 0777);

    mkdir((HomeDir + "/version").c_str(), 0777);
    ofstream MyLog(HomeDir + "/log.txt");
    ofstream MyStatus(HomeDir + "/status.txt");
    ofstream MyAdd(HomeDir + "/add.txt");
    MyLog.close();
    MyStatus.close();
    MyAdd.close();
    if (!check)
        cout << "git Initialized" << endl;
    else
        cout << "Re-initialising git" << endl;
    return;
}

void add(string file)
{
    vector<string> FilesToAdd;
    string dest = "./.git/add.txt";
    int i, j;
    vector<string> addFiles = file_reader(dest);
    unordered_set<string> addFilesSet;

    for (auto it : addFiles)
    {
        addFilesSet.insert(it);
    }

    if (file != ".")
    {
        if (dir_checker(file))
        {
            string folder = "./" + file;
            myls(&folder[0], FilesToAdd);
        }
        else
        {
            FilesToAdd.push_back(file);
        }

        for (i = 0; i < FilesToAdd.size(); i++)
        {
            if (addFilesSet.find(FilesToAdd[i]) != addFilesSet.end())
            {
                addFilesSet.erase(FilesToAdd[i]);
            }
        }
    }
    else
    {
        myls(".", FilesToAdd);
        for (auto it : FilesToAdd)
        {
            if (addFilesSet.find(it) != addFilesSet.end())
            {
                int h = 0;
                addFilesSet.erase(it);
            }
        }
    }
    for (auto it : addFilesSet)
        FilesToAdd.push_back(it);

    for (auto it : FilesToAdd)
    {
        ifstream ftemp(it);
        if (!ftemp)
        {
            string message = it + " : File does not exist";
            cout << message << "\n";
            trackerlogs("Added file did not exist\n");
            return;
        }
    }
    write_to_file(FilesToAdd, dest);
    cout << "Staging Successful\n";

    return;
}

void commitUtil(string cwd, vector<string> &pversion_files, unordered_map<string, string> statusFiles, int countD)
{
    if (countD != 0)
    {
        string folder = "./.git/version/v_" + to_string(countD);
        chdir(&folder[0]);
        vector<string> files;
        myls(".", pversion_files);
        chdir(&cwd[0]);
        for (auto it : pversion_files)
        {
            struct stat fileInfo;
            int h = 0;
            if (stat(&it[0], &fileInfo) != 0)
                statusFiles.erase(it);
            else
                files.push_back(it);
        }
        chdir(folder.c_str());
        int k = countD + 1;
        string dest = "../v_" + to_string(k);
        versionFolder_add(files, dest);
        chdir(cwd.c_str());
    }
    return;
}

string mycustom_msg(string msg)
{
    time_t rawtime;
    time(&rawtime);
    string text = "Commit\n#version: v_" + to_string(cnt_dir("./.git/version") + 1) + "\n" + "message: " + msg + "\ntimestamp: " + string(ctime(&rawtime)) + "\n\n";
    return text;
}

void modify_addFiles(vector<string> &addFiles, unordered_map<string, string> statusFiles)
{
    for (auto it : statusFiles)
    {
        string t = it.first;
        addFiles.push_back(t);
    }
    return;
}

void commit(string msg)
{
    if (!dir_checker("./.git"))
    {
        cout << "Git Not Initialized" << endl;
        return;
    }
    if (getFileSize("./.git/add.txt") == 0)
    {
        cout << "Nothing to commit" << endl;
        return;
    }

    string text = mycustom_msg(msg);
    int countD = cnt_dir("./.git/version");
    vector<string> allFiles;
    myls(".", allFiles);
    log_update(text);
    unordered_map<string, string> statusFiles = filemap_reader("./.git/status.txt");
    vector<string> addFiles = file_reader("./.git/add.txt");
    string destination = "./.git/version/v_" + to_string(countD + 1);
    int mk = mkdir(&destination[0], 0777);
    if (mk == -1)
    {
        perror("Version Directory not created");
        return;
    }
    char buff[PATH_MAX];
    getcwd(buff, PATH_MAX);
    string cwd = string(buff);

    unordered_set<string> allFilesSet;
    for (int i = 0; i < allFiles.size(); i++)
        allFilesSet.insert(allFiles[i]);

    vector<string> pversion_files;

    commitUtil(cwd, pversion_files, statusFiles, countD); // new

    versionFolder_add(addFiles, destination);
    vector<string> temp = addFiles;

    for (int i = 0; i < temp.size(); i++)
    {
        int h = 0;
        if (statusFiles.find(temp[i]) != statusFiles.end())
            statusFiles.erase(temp[i]);
    }

    modify_addFiles(addFiles, statusFiles);
    modify_status(addFiles);
    del_data("./.git/add.txt");
    return;
}

void printUntracked(vector<string> a)
{
    for (size_t i = 0; i < a.size(); i++)
    {
        cout << REDTEXT "(UNTRACKED)\t" << a[i] << RESET << endl;
    }
}

void printTbc(vector<string> a)
{
    for (size_t i = 0; i < a.size(); i++)
    {
        cout << GREENTEXT "(STAGED)\t" << a[i] << RESET << endl;
    }
}

void printModified(vector<string> a)
{
    for (size_t i = 0; i < a.size(); i++)
    {
        cout << YELLOWTEXT "(MODIFIED)\t" << a[i] << RESET << endl;
    }
}

void status()
{
    unordered_set<string> added_files;
    unordered_map<string, string> status_files;
    vector<string> files_tbc;
    vector<string> untracked;
    vector<string> modified;
    vector<string> all;

    myls(".", all);

    added_files = fileset_reader("./.git/add.txt");
    status_files = filemap_reader("./.git/status.txt");

    bool cond;

    for (auto fi : all)
    {
        string hash = SHA1::from_file(fi);
        if (added_files.find(fi) == added_files.end())
        {
            cond = true;
            if (status_files.find(fi) == status_files.end())
            {
                untracked.push_back(fi);
            }
        }
        if (added_files.find(fi) != added_files.end())
        {
            cond = false;
            files_tbc.push_back(fi);
        }
        if (status_files.find(fi) != status_files.end())
        {
            if (status_files[fi] == hash)
            {
                modified.push_back(fi);
            }
        }
    }

    printUntracked(untracked);
    printTbc(files_tbc);
    printModified(modified);
}

int myrmdir(const char *loc)
{
    int retval = -1;

    unsigned long path_len = strlen(loc);
    DIR *d = opendir(loc);

    if (d)
    {
        struct dirent *p;

        retval = 0;
        while (!retval && (p = readdir(d)))
        {
            if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, ".."))
                continue;

            char *temp_buffer;
            int rettemp = -1;
            unsigned long len = path_len;

            len += strlen(p->d_name);
            len++;
            len++;

            temp_buffer = new char[len];

            if (temp_buffer != NULL)
            {
                struct stat stb;

                snprintf(temp_buffer, len, "%s/%s", loc, p->d_name);

                if (!stat(temp_buffer, &stb))
                {
                    if (!S_ISDIR(stb.st_mode))
                        rettemp = unlink(temp_buffer);
                    else
                        rettemp = myrmdir(temp_buffer);
                }
                free(temp_buffer);
            }
            retval = rettemp;
        }

        closedir(d);
    }

    if (retval)
        return retval;
    else{
        retval = rmdir(loc);
        return retval;
    }    
    return 0;
}

bool mycustomcomparator(dirent *a, dirent *b)
{
    string s2 = string(b->d_name);
    string s1 = string(a->d_name);
    if (s1.compare(s2) >= 0)
    {
        return false;
    }
    return true;
}

void explore_dir(string path, vector<string> &dir_contents)
{
    DIR *d = opendir(path.c_str());
    int content_len = dir_contents.size();
    struct dirent *entity = readdir(d);
    
    dir_contents.clear();
    while (entity != NULL)
    {
        string content = entity->d_name;
        dir_contents.push_back(content);
        entity = readdir(d);
    }
    closedir(d);
    sort(dir_contents.begin(), dir_contents.end());
}

void cpfile2(string source, string destination, char *d_name)
{
    size_t size;
    if (current_directory.find(string(d_name)) != current_directory.end())
    {
        del_file(&(destination)[0]);
    }
    FILE *dest1 = fopen(&destination[0], "w");
    FILE *src1 = fopen(&source[0], "r");
    
    char buffertemp[BUFSIZ];
    size = fread(buffertemp, 1, BUFSIZ, src1);

    while (size > 0)
    {
        fwrite(buffertemp, 1, size, dest1);
        size = fread(buffertemp, 1, BUFSIZ, src1);
    }

    struct stat point;
    stat(source.c_str(), &point);
    chmod(destination.c_str(), point.st_mode);
    
    fclose(dest1);
    fclose(src1);
}

void cpdir2(const char *source, const char *destination, char *d_name)
{
    if (current_directory.find(string(d_name)) != current_directory.end())
    {
        myrmdir(&(string(destination) + '/' + string(d_name))[0]);
    }

    DIR *x;
    struct dirent *d1;
    x = opendir(source);
    int status = mkdir(destination, 0777);
    if (x)
    {
        while ((d1 = readdir(x)) != NULL)
        {
            if ((string(d1->d_name) == ".") || (string(d1->d_name) == ".."))
                continue;
            else
            {
                string destpath = string(destination);
                destpath += "/";
                destpath += string(d1->d_name);

                int len2 = destpath.length() + 1;
                const char *destp = new char[len2];

                string tempd = destpath;
                destp = tempd.c_str();


                string sourcepath = string(source);
                sourcepath += "/";
                sourcepath += string(d1->d_name);

                int len1 = sourcepath.length() + 1;
                const char *newp = new char[len1];

                string tempn = sourcepath;
                newp = tempn.c_str();


                struct stat point;

                if (stat(newp, &point) != -1)
                {
                    if ((S_ISDIR(point.st_mode)))
                    {
                        cpdir2(newp, destp, d1->d_name);
                    }
                    else
                    {
                        cpfile2(newp, destp, d1->d_name);
                    }
                }
                else
                {
                    cout<<"Error encountered while copying directory\n";
                    trackerlogs("Error encountered while copying directory\n");
                }
            }
        }
    }
    else
    {
        cout << "Something went wrong while copying the version\n";
        trackerlogs("Something went wrong while copying the version\n");
    }
}

void cp_ver(const char *source, const char *des)
{
    struct dirent *dir;
    DIR *x;
    if (x = opendir(source))
    {
        while ((dir = readdir(x)) != NULL)
        {
            if ((string(dir->d_name) == ".") || (string(dir->d_name) == ".."))
                continue;
            else
            {
                string sourcepath = string(source);
                sourcepath += "/";
                sourcepath += string(dir->d_name);

                int len1 = sourcepath.length() + 1;
                const char *newp = new char[len1];
                string tempn = sourcepath;
                newp = tempn.c_str();

                string destpath = string(des);
                destpath += "/";
                destpath += string(dir->d_name);

                const char *destp = new char[destpath.length() + 1];
                string tempd = destpath;
                destp = tempd.c_str(); 

                struct stat point;
                if (stat(newp, &point) != -1)
                {
                     if ((S_ISDIR(point.st_mode)))
                    {
                        cpdir2(newp, destp, dir->d_name);
                    }
                    else
                    {
                        cpfile2(newp, destp, dir->d_name);
                    }
                }
                else
                {
                    cout<<"Error encountered while copying version\n";
                    trackerlogs("Error encountered while copying version\n");
                   
                }
            }
        }
    }
    else
    {
        cout << "Something went wrong while copying the version\n";
        trackerlogs("Something went wrong while copying the version\n");
    }
}

void rmfolder(vector<string> arr)
{
    unsigned int i=0;
    int mylen = arr.size();
    while(i<mylen)
    {
        if (arr[i] == "trackerlog.txt" || arr[i] == "README.md" || arr[i] == "." || arr[i] == "git" || arr[i] == ".git" || arr[i] == "git.cpp" || arr[i] == ".." || arr[i] == "sha1.hpp")
        {
            i++;
            continue;
        }
        string path = arr[i];
        if (dir_checker(path))
        {
            vector<string> dirinfo;
            DIR *c = opendir(&path[0]);
            struct dirent *arr;
            while ((arr = readdir(c)) != NULL)
            {
                dirinfo.push_back((string)arr->d_name);
            }

            sort(dirinfo.begin(), dirinfo.end());
            closedir(c);

            int j=0,mylen = dirinfo.size();
            while(j<mylen)
            {
                if (dirinfo[j] == ".." || dirinfo[j] == "."){
                    j++;
                    continue;
                }
                else{
                    dirinfo[j] = path + "/" + dirinfo[j];
                    j++;
                }
            }
            rmfolder(dirinfo);
            rmdir(path.c_str());
        }
        else
        {
            remove(path.c_str());
        }
        i++;
    }
}

void rollback()
{

    DIR *df;
    df = opendir(".git/version");

    if (df)
    {
        dirfiles.clear();
        DIRENT *q;
        DIRENT *p;

        while ((p = readdir(df)) != NULL)
        {
            if (p->d_name != "." && p->d_name != "..")
            {
                dirfiles.push_back(p);
            }
        }
        sort(dirfiles.begin(), dirfiles.end(), mycustomcomparator);

        string version_to_remove, final_source;
        version_to_remove = dirfiles[dirfiles.size() - 1]->d_name;

        current_directory.clear();

        char path[PATH_MAX];
        getcwd(path, PATH_MAX);
        DIR *e = opendir(path);

        if (e != NULL)
        {
            q = readdir(e);
            while (q != NULL)
            {
                current_directory.insert(q->d_name);
                q = readdir(e);
            }
        }

        string finalsource_revert = "./git";
        final_source = ".git/version/" + version_to_remove;

        string rollback_ver, rollback_path;
        myrmdir(&final_source[0]);

        dirfiles.pop_back();
        unsigned int i;
        i=0;
        int mylen = dirfiles.size();
        while(i<mylen)
        {
            if (((string)dirfiles[i]->d_name != "..") && ((string)dirfiles[i]->d_name != "."))
            {
                if (dirfiles[i]->d_name > rollback_ver)
                    rollback_ver = dirfiles[i]->d_name;
            }
            else{
                //pass
            }
            i++;
        }

        rollback_path = ".git/version/" + rollback_ver;
        char buff[PATH_MAX];
        getcwd(buff, PATH_MAX);
        string cwd = string(buff);

        vector<string> dirinfo;
        DIR *c = opendir(&cwd[0]);
        struct dirent *files;
        while ((files = readdir(c)) != NULL)
        {
            string name = files->d_name;
            dirinfo.push_back(name);
        }
        closedir(c);
        sort(dirinfo.begin(), dirinfo.end());

         rmfolder(dirinfo);
        vector<string> prev_contents;
        cp_ver(&rollback_path[0], path);

        chdir(&rollback_path[0]);
        myls(".", prev_contents);

        versionFolder_add(prev_contents, "../../../");
        chdir(&cwd[0]);
    }
    else
    {
        cout << "Version folder does not exist. Run ./git init\n";
    }
}

void log()
{
    cout << endl;
    vector<string> t = file_reader("./.git/log.txt");
    for (int i = 0; i < t.size(); i++)
    {
        cout << t[i] << endl;
    }
    return;
}

bool init_first()
{
    if (!dir_checker("./.git"))
    {
        cout << "Git Not Initialized" << endl;
        return true;
    }
    return false;
}

void menu(vector<string> args)
{
    string cmd = args[0];
    if (cmd == "init")
    {
        init();
        trackerlogs("Init Folder created\n");
    }
    else if (cmd == "add")
    {
        if (init_first())
        {
            trackerlogs("git add failed as init was not created\n");
            return;
        }
        else if (args.size() != 2)
        {
            cout << "Invalid number of arguments\nTip: For adding all files use add .\n";
        }
        else
        {
            string file = args[1];
            add(file);
        }
    }
    else if (cmd == "commit")
    {
        if (init_first())
        {
            trackerlogs("git commit failed as init was not created\n");
            return;
        }
        string msg = "";
        bool tf = true;
        if (args.size() > 1)
        {
            if (args[1] == "-m")
            {
                for (size_t i = 2; i < args.size(); i++)
                {
                    msg += args[i];
                    msg += " ";
                }
            }
            else
            {
                tf = false;
                cout << "Invalid command\n";
            }
        }
        if (tf)
        {
            commit(msg);
            trackerlogs("Commit successful\n");
            cout << "Commit Successful\n";
        }
    }
    else if (cmd == "status")
    {
        if (init_first())
        {
            trackerlogs("git status failed as init was not created\n");
            return;
        }
        status();
        trackerlogs("git status viewed\n");
    }
    else if (cmd == "rollback")
    {
        if (init_first())
        {
            trackerlogs("git rollback failed as init was not created\n");
            return;
        }
        int countDir = cnt_dir("./.git/version");
        if (countDir == 0)
        {
            cout << "No version to rollback to\n";
            trackerlogs("Invalid git rollback\n");
        }
        else if (args.size() == 1)
        {
            rollback();
            time_t rawtime;
            time(&rawtime);
            string text = "Rollback\n";
            text += "Rollback\n#version: v_" + to_string(countDir) + " => v_" + to_string(countDir - 1) + "\n";
            text += "timestamp: " + string(ctime(&rawtime)) + "\n\n";
            log_update(text);
            cout << "Rollback to previous version completed successfully\n";
            trackerlogs("Rollback (without arguments) to previous version completed successfully\n");
        }
        else if (args.size() == 3)
        {
            if (args[1] == "-c") // rollback by checkpoint (rollback '1', rollback '2', etc)
            { 
                int ver = stoi(args[2]);
                if (countDir < ver || ver < 1)
                {
                    cout << "Invalid version rollback\n";
                    trackerlogs("Invalid git rollback\n");
                }
                else
                {
                    for (int i = 0; i < ver; i++)
                    {
                        rollback();
                    }
                    time_t rawtime;
                    time(&rawtime);
                    string text = "Rollback\n";
                    text += "#version: v_" + to_string(countDir) + " => v_" + to_string(countDir - ver) + "\n";
                    text += "timestamp: " + string(ctime(&rawtime)) + "\n\n";
                    log_update(text);
                    cout << "Rollback to " << ver << " versions completed successfully\n";
                    trackerlogs("Rollback (with -c argument) to previous version completed successfully\n");
                }
            }
            else if (args[1] == "-vn") // rollback by version name (rollback 'V2', rollback 'V1', etc)
            {
                string v = args[2].substr(1, args[2].size());
                int ver = stoi(v);
                if (countDir < ver || ver < 1)
                {
                    cout << "Invalid version rollback\n";
                    trackerlogs("Invalid git rollback\n");
                }
                else
                {
                    for (int i = countDir; i > ver; i--)
                    {
                        rollback();
                    }
                    time_t rawtime;
                    time(&rawtime);
                    string text = "Rollback\n";
                    text += "#version: v_" + to_string(countDir) + " => v_" + to_string(ver) + "\n";
                    text += "timestamp: " + string(ctime(&rawtime)) + "\n\n";
                    log_update(text);
                    cout << "Rollback to version " << args[2] << " completed successfully\n";
                    trackerlogs("Rollback (with -vn argument) to previous version completed successfully\n");
                }
            }
        }
        else
        {
            cout << "Invalid number of arguments\n";
            trackerlogs("Invalid number of arguments in git rollback\n");
        }
    }
    else if (cmd == "log")
    {
        if (init_first())
        {
            trackerlogs("git log failed as init was not created\n");
            return;
        }
        log();
    }
    else
    {
        trackerlogs("Invalid comment was enetered\n");
        cout << "Invalid git command\n";
    }
}

int main(int argc, char *argv[])
{

    if (argc == 1)
    {
        cout << "Enter valid arguments" << endl;
        return 0;
    }
    vector<string> args;
    for (int i = 1; i < argc; i++)
    {
        args.push_back(argv[i]);
    }
    menu(args);
    return 0;
}

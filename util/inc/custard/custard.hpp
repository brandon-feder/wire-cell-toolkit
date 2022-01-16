/**
   custard

   Provide support for tar header codec.
*/

#ifndef custard_hpp
#define custard_hpp

#include <istream>
#include <cstring>
#include <ctime>
#include <cstdlib>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <sstream>
#include <algorithm>

#include <iostream>
#include <cassert>

// This and other guidance from
// https://techoverflow.net/2013/03/29/reading-tar-files-in-c/
// Converts an ascii digit to the corresponding number
#define ASCII_TO_NUMBER(num) ((num)-48) 

namespace custard {


    inline
    void encode_chksum(char* var, int64_t val) {
        std::stringstream ss;
        ss << std::oct << val;
        std::string s = ss.str();
        if (s[0] != '0' and s.size() < 8) {
            s.insert(0, "0");
        }
        if (s.size() % 2) {
            s.insert(0, "0");
        }
        if (s.size() < 8) {
            s += '\0';
        }
        while (s.size() < 8) {
            s += ' ';
        }
        memcpy(var, s.data(), 8);
    }
    inline
    void encode_null(char* var, int64_t val, size_t odigits) {
        std::stringstream ss;
        ss << std::oct << val;
        std::string s = ss.str();
        while (s.size() < odigits-1) {
            s.insert(0, "0");
        }
        s += '\0';
        memcpy(var, s.data(), odigits);
    }
    inline
    std::string encode_octal(int64_t val, size_t odigits) {
        std::stringstream ss;
        ss << std::oct << val;
        std::string s = ss.str();
        while (s.size() < odigits) {
            s.insert(0, "0");
        }
        return s;
    }

    inline
    void encode_octal_copy(char* var, int64_t val, size_t odigits) {
        auto s = encode_octal(val, odigits);
        memcpy(var, s.data(), odigits);
    }

    inline
    uint64_t decode_octal(const char* data, size_t size) {
        unsigned char* currentPtr = (unsigned char*) data + size;
        uint64_t sum = 0;
        uint64_t currentMultiplier = 1;
        unsigned char* checkPtr = currentPtr;

        for (; checkPtr >= (unsigned char*) data; checkPtr--) {
            if ((*checkPtr) == 0 || (*checkPtr) == ' ') {
                currentPtr = checkPtr - 1;
            }
        }
        for (; currentPtr >= (unsigned char*) data; currentPtr--) {
            sum += ASCII_TO_NUMBER(*currentPtr) * currentMultiplier;
            currentMultiplier *= 8;
        }
        return sum;
    }

    class Header
    {                         /* byte offset */
        char name_[100];               /*   0 */
        char mode_[8];                 /* 100 */
        char uid_[8];                  /* 108 */
        char gid_[8];                  /* 116 */
        char size_[12];                /* 124 */
        char mtime_[12];               /* 136 */
        char chksum_[8];               /* 148 */
        char typeflag_;                /* 156 */
        char linkname_[100];           /* 157 */
        char magic_[6];                /* 257 */
        char version_[2];              /* 263 */
        char uname_[32];               /* 265 */
        char gname_[32];               /* 297 */
        char devmajor_[8];             /* 329 */
        char devminor_[8];             /* 337 */
        char prefix_[155];             /* 345 */
        char padding_[12];             /* 500 */
                                       /* 512 */
      public:
        Header(std::string filename="", size_t siz=0)
        {
            init(filename, siz);
        }
        void clear()
        {
            memset((char*)this, '\0', 512);
        }
        void init(std::string filename="", size_t siz=0)
        {
            clear();
            set_name(filename);
            set_mode();
            auto uid = geteuid();
            set_uid(uid);
            auto gid = getegid();
            set_gid(gid);
            set_size(siz);
            set_mtime(time(0));
            typeflag_ = '0';
            memcpy(magic_, "ustar ", 6);
            // old gnu magic over writes version.
            version_[0] = ' ';  
            auto* pw = getpwuid (uid);
            if (pw) {
                set_uname(pw->pw_name);
            }
            auto* gw = getgrgid(gid);
            if (gw) {
                set_gname(gw->gr_name);
            }
            set_chksum(checksum());
        }

        void set_name(const std::string& name)
        {
            memcpy(name_, name.data(), std::min(name.size(), 100UL));
        }
        std::string name() const {
            return name_;
        }

        void set_mode(int mode=0644)
        {
            encode_null(mode_, mode, 8);
        }
        size_t mode() const
        {
            return decode_octal(mode_, 8);
        }

        void set_mtime(size_t mtime)
        {
            encode_null(mtime_, mtime, 12);
        }
        size_t mtime() const
        {
            return decode_octal(mtime_, 12);
        }

        void set_uid(size_t id)
        {
            encode_null(uid_, id, 8);
        }
        size_t uid() const
        {
            return decode_octal(uid_, 8);
        }

        void set_gid(size_t id)
        {
            encode_null(gid_, id, 8);
        }
        size_t gid() const
        {
            return decode_octal(gid_, 8);
        }

        void set_uname(const std::string& uname)
        {
            memcpy(uname_, uname.data(), std::min(uname.size(), 32UL));
        }
        std::string uname() const
        {
            return uname_;
        }

        void set_gname(const std::string& gname)
        {
            memcpy(gname_, gname.data(), std::min(gname.size(), 32UL));
        }
        std::string gname() const
        {
            return gname_;
        }

        /// Note, this is the size of the body data, NOT this header
        /// (which is always a fixed 512 bytes).
        void set_size(size_t size)
        {
            encode_null(size_, size, 12);
        }
        size_t size() const {
            return decode_octal(size_, 12);
        }


        uint32_t chksum() const {
            return (uint32_t) decode_octal(chksum_, 8);
        }

        // Calculate and return checksum of header
        uint32_t checksum() const {
            uint32_t usum{0};
            for (size_t ind = 0; ind < 148; ++ind) {
                usum += ((unsigned char*) this)[ind];
            }
            // the checksum field itself is must be interpreted as
            // spaces.
            for (size_t ind = 0; ind < 8; ++ind) {
                usum += ((unsigned char) ' ');
            }                
            for (size_t ind = 156; ind < 512; ++ind) {
                usum += ((unsigned char*) this)[ind];
            }
            return usum;
        }

        void set_chksum(uint32_t usum) {
            encode_chksum(chksum_, usum);
        }

        void gensum() {
            set_chksum(checksum());
        }

        bool is_ustar() const {
            return memcmp("ustar", magic_, 5) == 0;
        }

        char* as_bytes()  {
            return &name_[0];
        }
        const char* as_bytes() const {
            return &name_[0];
        }

        // the amount of padding expected at end of file.
        size_t padding() const {
            return 512UL - size()%512UL;
        }

        /// Read self from stream as tar header (not custard stream!)
        std::istream& read(std::istream& si) {
            return si.read(&name_[0], 512);
        }

        /// Advance the stream beyond the padding at the end of file
        /// data.
        std::istream& read_pad(std::istream& si) {
            si.seekg(padding(), si.cur);
            return si;
        }

        /// Write self to stream as tar header (not custard stream!)
        std::ostream& write(std::ostream& so) {
            return so.write(&name_[0], 512);
        }
            
        /// Write the padding that goes after file data
        std::ostream& write_pad(std::ostream& so)
        {
            std::string pad(padding(), 0);
            so.write(&pad[0], pad.size());
            return so;
        }


    };

}
#endif


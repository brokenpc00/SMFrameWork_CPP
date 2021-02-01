//
//  StringUtil.cpp
//  SMFrameWork
//
//  Created by KimSteve on 2016. 11. 3..
//
//

#include "StringUtil.h"

#define BUFF_SIZE 1024

class comma_numpunct : public std::numpunct<char>
{
public:
    comma_numpunct() : decimalChar('.'), groupChar(','), groupingCnt("\03")
    {
        
    };
    ~comma_numpunct(){};
    char decimalChar;
    char groupChar;
    std::string groupingCnt;
protected:
    virtual char do_decimal_point()
    {
        return decimalChar;
    }
    
    virtual char do_thousands_sep() const
    {
        return groupChar;
    }
    
    virtual std::string do_grouping() const
    {
        return groupingCnt;
    }
};


void StringUtil::split(const std::string& str, const std::string& delim, std::vector<std::string>& elems) {
    if (!str.empty()) {
        size_t nend = 0;
        size_t nbegin = 0;
        size_t delimSize = delim.size();
        size_t len = str.length();
        
        while(nend != std::string::npos) {
            nend = str.find(delim, nbegin);
            if(nend == std::string::npos) {
                elems.push_back(str.substr(nbegin, len-nbegin));
            } else {
                elems.push_back(str.substr(nbegin, nend-nbegin));
            }
            
            nbegin = nend + delimSize;
        }
    }
}

std::vector<std::string> StringUtil::split(const std::string& str, const std::string& delim) {
    std::vector<std::string> elems;
    split(str, delim, elems);
    return elems;
}

uint32_t StringUtil::hexToColor(const std::string& str) {
    if (str.front() == '#') {
        size_t len = str.length();
        if (len == 7) {
            int64_t value = strtoll(str.substr(1).c_str(), nullptr, 16);
            if (value >= 0 && value <= 0xffffff) {
                return (uint32_t)(0xff000000 | value);
            }
        } else if (len == 9) {
            int64_t value = strtoll(str.substr(1).c_str(), nullptr, 16);
            if (value >= 0 && value <= 0xffffffff) {
                return (uint32_t)value;
            }
        }
    }
    
    return 0;
}

/*
 // http://rextester.com/WMK79392
 std::string StringUtil::format(const char* format, ...) {
 std::string ret;
 
 va_list args;
 va_start(args, format);
 {
 size_t size = std::snprintf(nullptr, 0, format, args) + 1; // Extra space for '\0'
 std::unique_ptr<char[]> buf(new char[size]);
 std::vsnprintf(buf.get(), size, format, args);
 
 ret = std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside
 }
 va_end(args);
 
 return ret;
 }
 */

bool StringUtil::compareIgnoreCase(const std::string& src, const std::string& dst) {
    if (src.size() != dst.size())
        return false;
    
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS)
	return 0 == strncmp(src.c_str(), dst.c_str(), src.size());
#else
    return 0 == ::strncasecmp(src.c_str(), dst.c_str(), src.size());
#endif

	
}

bool StringUtil::startsWith(const std::string& str, const std::string& prefix) {
    return 0 == str.compare(0, prefix.size(), prefix);
}

bool StringUtil::startsWithIgnoreCase(const std::string& str, const std::string& prefix) {
    auto str2 = toLower(str);
    auto prefix2 = toLower(prefix);
    
    return 0 == str2.compare(0, prefix2.size(), prefix2);
}

std::string StringUtil::toUpper(const std::string& str) {
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::toupper);
    
    return s;
}

std::string StringUtil::toLower(const std::string& str) {
    std::string s(str);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    
    return s;
}

/*
 void StringUtil::ltrim(std::string& str) {
 str.erase(str.begin(), std::find_if(str.begin(), str.end(),
 std::not1(std::ptr_fun<int, int>(std::isspace))));
 }
 
 void StringUtil::rtrim(std::string& str) {
 str.erase(std::find_if(str.rbegin(), str.rend(),
 std::not1(std::ptr_fun<int, int>(std::isspace))).base(), str.end());
 }
 
 void StringUtil::trim(std::string& str) {
 ltrim(str);
 rtrim(str);
 }
 */
std::string& StringUtil::ltrim(std::string& s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));

    return s;
}

std::string& StringUtil::rtrim(std::string& s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

std::string& StringUtil::trim(std::string& s) {
    return StringUtil::ltrim(StringUtil::rtrim(s));
}

std::string StringUtil::replaceAll(const std::string& str, const std::string& from, const std::string& to) {
    std::string s(str);
    
    if (!s.empty()) {
        size_t start_pos = 0;
        
        while ((start_pos = s.find(from, start_pos)) != std::string::npos) {
            s.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }
    
    return s;
}



//http://flylib.com/books/en/2.878.1.47/1/
static int spc_email_isvalid(const char *address) {
    int count = 0;
    const char *c, *domain;
    const char *rfc822_specials = "()<>@,;:\\\"[]";
    
    /* first we validate the name portion (name@domain) */
    for (c = address; *c; c++) {
        if (*c == '\"' && (c == address || *(c - 1) == '.' || *(c - 1) ==  '\"')) {
            while (*++c) {
                if (*c == '\"')
                    break;
                
                if (*c == '\\' && (*++c == ' '))
                    continue;
                
                if (*c <= ' ' || *c >= 127)
                    return 0;
            }
            
            if (!*c++)
                return 0;
            
            if (*c == '@') break;
            if (*c != '.') return 0;
            
            continue;
        }
        
        if (*c == '@')
            break;
        
        if (*c <= ' ' || *c >= 127)
            return 0;
        
        if (strchr(rfc822_specials, *c))
            return 0;
    }
    
    if (c == address || *(c - 1) == '.')
        return 0;
    
    /* next we validate the domain portion (name@domain) */
    if (!*(domain = ++c))
        return 0;
    
    do {
        if (*c == '.') {
            if (c == domain || *(c - 1) == '.')
                return 0; count++;
        }
        if (*c <= ' ' || *c >= 127) return 0;
        if (strchr(rfc822_specials, *c))
            return 0;
    } while (*++c);
    
    return (count >= 1);
}


bool StringUtil::checkEmailIsValid(const std::string& str) {
    int ret = spc_email_isvalid(str.c_str());
    
    return ret > 0;
}

int StringUtil::toInt(const std::string& str) {
    if (!str.empty() && isDigits(str)) {
        return std::atoi(str.c_str());
    }
    return 0;
}

long StringUtil::toLong(const std::string& str) {
    if (!str.empty() && isDigits(str)) {
        return std::atol(str.c_str());
    }
    return 0;
}

bool StringUtil::isDigits(const std::string &str) {
    return std::all_of(str.begin(), str.end(), ::isdigit); // C++11
}

int64_t StringUtil::toInt64(const std::string& str) {
    return atoll(str.c_str());
}

std::string StringUtil::getCurrencyStringFromNumeric(int64_t decimal)
{
    comma_numpunct * numP = new comma_numpunct();
    numP->decimalChar = '.';
    numP->groupChar = ',';
    numP->groupingCnt = "\03";
    std::locale comma_locale(std::locale(), numP);
    std::stringstream ss;
    ss.imbue(comma_locale);
    
    ss << std::setprecision(2) << std::fixed << decimal;
    
    return ss.str();
}

std::vector<std::string> StringUtil::separateStringByString(const std::string str, const char *delimiter)
{
    std::vector<std::string> retVec;
    size_t cutAt;
    
    std::string strTmp = str;
    
    while ((cutAt=strTmp.find_first_of(delimiter)) != strTmp.npos ) {
        if (cutAt>0) {
            retVec.push_back(str.substr(0, cutAt));
        }
        strTmp = strTmp.substr(cutAt+1);
    }
    
    if (!strTmp.empty()) {
        retVec.push_back(strTmp);
    }
    
    return retVec;
}

std::string StringUtil::getYYYYMMDDString(time_t date)
{
    time_t newDateTime = date;
    char timeStamp[256];
    strftime(timeStamp, sizeof timeStamp, "%Y%m%d", gmtime(&newDateTime));
    std::string ts(timeStamp);
    return ts;
}

std::string StringUtil::getTodayDateString()
{
    time_t now;
    time (&now);

    char timeStamp[256];
    strftime(timeStamp, BUFSIZ, "%Y년 %m월 %d일", localtime(&now));
    
    std::string ts(timeStamp);
    
    return ts;
}

std::string StringUtil::getTimeString(time_t dateTime)
{
    time_t newDateTime = dateTime;
    char timeStamp[256];
    strftime(timeStamp, sizeof timeStamp, "%H%M", gmtime(&newDateTime));
    std::string ts(timeStamp);
    return ts;
}

std::string StringUtil::getMDayString(time_t dateTime)
{
    time_t newDateTime = dateTime;
    char timeStamp[256];
    strftime(timeStamp, sizeof timeStamp, "%m.%d", gmtime(&newDateTime));
    std::string ts(timeStamp);
    return ts;
}

std::string StringUtil::getCurrentTimeString()
{
    time_t now;
    time (&now);
    
    char timeStamp[256];
    
    strftime(timeStamp, BUFSIZ, "%p %l시 %M분", localtime(&now));
    
    std::string ts(timeStamp);

    return ts;
}

std::string StringUtil::getDateString(time_t date)
{
    time_t newDateTime = date;
    char timeStamp[256];
    strftime(timeStamp, sizeof timeStamp, "%d", gmtime(&newDateTime));
    std::string ts(timeStamp);
    return ts;
}

std::string StringUtil::getDateCompareString(time_t date)
{
    time_t newDateTime = date;
    char timeStamp[256];
    strftime(timeStamp, sizeof timeStamp, "%x", gmtime(&newDateTime));
    std::string ts(timeStamp);
    return ts;
}


double StringUtil::atof(const char *nptr)
{
    return (strtod(nptr, NULL));
}


#include <errno.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define UNICODE_MAX 0x10FFFFul

static const char *const NAMED_ENTITIES[][2] = {
    { "AElig;", "Æ" },
    { "Aacute;", "Á" },
    { "Acirc;", "Â" },
    { "Agrave;", "À" },
    { "Alpha;", "Α" },
    { "Aring;", "Å" },
    { "Atilde;", "Ã" },
    { "Auml;", "Ä" },
    { "Beta;", "Β" },
    { "Ccedil;", "Ç" },
    { "Chi;", "Χ" },
    { "Dagger;", "‡" },
    { "Delta;", "Δ" },
    { "ETH;", "Ð" },
    { "Eacute;", "É" },
    { "Ecirc;", "Ê" },
    { "Egrave;", "È" },
    { "Epsilon;", "Ε" },
    { "Eta;", "Η" },
    { "Euml;", "Ë" },
    { "Gamma;", "Γ" },
    { "Iacute;", "Í" },
    { "Icirc;", "Î" },
    { "Igrave;", "Ì" },
    { "Iota;", "Ι" },
    { "Iuml;", "Ï" },
    { "Kappa;", "Κ" },
    { "Lambda;", "Λ" },
    { "Mu;", "Μ" },
    { "Ntilde;", "Ñ" },
    { "Nu;", "Ν" },
    { "OElig;", "Œ" },
    { "Oacute;", "Ó" },
    { "Ocirc;", "Ô" },
    { "Ograve;", "Ò" },
    { "Omega;", "Ω" },
    { "Omicron;", "Ο" },
    { "Oslash;", "Ø" },
    { "Otilde;", "Õ" },
    { "Ouml;", "Ö" },
    { "Phi;", "Φ" },
    { "Pi;", "Π" },
    { "Prime;", "″" },
    { "Psi;", "Ψ" },
    { "Rho;", "Ρ" },
    { "Scaron;", "Š" },
    { "Sigma;", "Σ" },
    { "THORN;", "Þ" },
    { "Tau;", "Τ" },
    { "Theta;", "Θ" },
    { "Uacute;", "Ú" },
    { "Ucirc;", "Û" },
    { "Ugrave;", "Ù" },
    { "Upsilon;", "Υ" },
    { "Uuml;", "Ü" },
    { "Xi;", "Ξ" },
    { "Yacute;", "Ý" },
    { "Yuml;", "Ÿ" },
    { "Zeta;", "Ζ" },
    { "aacute;", "á" },
    { "acirc;", "â" },
    { "acute;", "´" },
    { "aelig;", "æ" },
    { "agrave;", "à" },
    { "alefsym;", "ℵ" },
    { "alpha;", "α" },
    { "amp;", "&" },
    { "and;", "∧" },
    { "ang;", "∠" },
    { "apos;", "'" },
    { "aring;", "å" },
    { "asymp;", "≈" },
    { "atilde;", "ã" },
    { "auml;", "ä" },
    { "bdquo;", "„" },
    { "beta;", "β" },
    { "brvbar;", "¦" },
    { "bull;", "•" },
    { "cap;", "∩" },
    { "ccedil;", "ç" },
    { "cedil;", "¸" },
    { "cent;", "¢" },
    { "chi;", "χ" },
    { "circ;", "ˆ" },
    { "clubs;", "♣" },
    { "cong;", "≅" },
    { "copy;", "©" },
    { "crarr;", "↵" },
    { "cup;", "∪" },
    { "curren;", "¤" },
    { "dArr;", "⇓" },
    { "dagger;", "†" },
    { "darr;", "↓" },
    { "deg;", "°" },
    { "delta;", "δ" },
    { "diams;", "♦" },
    { "divide;", "÷" },
    { "eacute;", "é" },
    { "ecirc;", "ê" },
    { "egrave;", "è" },
    { "empty;", "∅" },
    { "emsp;", "\xE2\x80\x83" },
    { "ensp;", "\xE2\x80\x82" },
    { "epsilon;", "ε" },
    { "equiv;", "≡" },
    { "eta;", "η" },
    { "eth;", "ð" },
    { "euml;", "ë" },
    { "euro;", "€" },
    { "exist;", "∃" },
    { "fnof;", "ƒ" },
    { "forall;", "∀" },
    { "frac12;", "½" },
    { "frac14;", "¼" },
    { "frac34;", "¾" },
    { "frasl;", "⁄" },
    { "gamma;", "γ" },
    { "ge;", "≥" },
    { "gt;", ">" },
    { "hArr;", "⇔" },
    { "harr;", "↔" },
    { "hearts;", "♥" },
    { "hellip;", "…" },
    { "iacute;", "í" },
    { "icirc;", "î" },
    { "iexcl;", "¡" },
    { "igrave;", "ì" },
    { "image;", "ℑ" },
    { "infin;", "∞" },
    { "int;", "∫" },
    { "iota;", "ι" },
    { "iquest;", "¿" },
    { "isin;", "∈" },
    { "iuml;", "ï" },
    { "kappa;", "κ" },
    { "lArr;", "⇐" },
    { "lambda;", "λ" },
    { "lang;", "〈" },
    { "laquo;", "«" },
    { "larr;", "←" },
    { "lceil;", "⌈" },
    { "ldquo;", "“" },
    { "le;", "≤" },
    { "lfloor;", "⌊" },
    { "lowast;", "∗" },
    { "loz;", "◊" },
    { "lrm;", "\xE2\x80\x8E" },
    { "lsaquo;", "‹" },
    { "lsquo;", "‘" },
    { "lt;", "<" },
    { "macr;", "¯" },
    { "mdash;", "—" },
    { "micro;", "µ" },
    { "middot;", "·" },
    { "minus;", "−" },
    { "mu;", "μ" },
    { "nabla;", "∇" },
    { "nbsp;", "\xC2\xA0" },
    { "ndash;", "–" },
    { "ne;", "≠" },
    { "ni;", "∋" },
    { "not;", "¬" },
    { "notin;", "∉" },
    { "nsub;", "⊄" },
    { "ntilde;", "ñ" },
    { "nu;", "ν" },
    { "oacute;", "ó" },
    { "ocirc;", "ô" },
    { "oelig;", "œ" },
    { "ograve;", "ò" },
    { "oline;", "‾" },
    { "omega;", "ω" },
    { "omicron;", "ο" },
    { "oplus;", "⊕" },
    { "or;", "∨" },
    { "ordf;", "ª" },
    { "ordm;", "º" },
    { "oslash;", "ø" },
    { "otilde;", "õ" },
    { "otimes;", "⊗" },
    { "ouml;", "ö" },
    { "para;", "¶" },
    { "part;", "∂" },
    { "permil;", "‰" },
    { "perp;", "⊥" },
    { "phi;", "φ" },
    { "pi;", "π" },
    { "piv;", "ϖ" },
    { "plusmn;", "±" },
    { "pound;", "£" },
    { "prime;", "′" },
    { "prod;", "∏" },
    { "prop;", "∝" },
    { "psi;", "ψ" },
    { "quot;", "\"" },
    { "rArr;", "⇒" },
    { "radic;", "√" },
    { "rang;", "〉" },
    { "raquo;", "»" },
    { "rarr;", "→" },
    { "rceil;", "⌉" },
    { "rdquo;", "”" },
    { "real;", "ℜ" },
    { "reg;", "®" },
    { "rfloor;", "⌋" },
    { "rho;", "ρ" },
    { "rlm;", "\xE2\x80\x8F" },
    { "rsaquo;", "›" },
    { "rsquo;", "’" },
    { "sbquo;", "‚" },
    { "scaron;", "š" },
    { "sdot;", "⋅" },
    { "sect;", "§" },
    { "shy;", "\xC2\xAD" },
    { "sigma;", "σ" },
    { "sigmaf;", "ς" },
    { "sim;", "∼" },
    { "spades;", "♠" },
    { "sub;", "⊂" },
    { "sube;", "⊆" },
    { "sum;", "∑" },
    { "sup1;", "¹" },
    { "sup2;", "²" },
    { "sup3;", "³" },
    { "sup;", "⊃" },
    { "supe;", "⊇" },
    { "szlig;", "ß" },
    { "tau;", "τ" },
    { "there4;", "∴" },
    { "theta;", "θ" },
    { "thetasym;", "ϑ" },
    { "thinsp;", "\xE2\x80\x89" },
    { "thorn;", "þ" },
    { "tilde;", "˜" },
    { "times;", "×" },
    { "trade;", "™" },
    { "uArr;", "⇑" },
    { "uacute;", "ú" },
    { "uarr;", "↑" },
    { "ucirc;", "û" },
    { "ugrave;", "ù" },
    { "uml;", "¨" },
    { "upsih;", "ϒ" },
    { "upsilon;", "υ" },
    { "uuml;", "ü" },
    { "weierp;", "℘" },
    { "xi;", "ξ" },
    { "yacute;", "ý" },
    { "yen;", "¥" },
    { "yuml;", "ÿ" },
    { "zeta;", "ζ" },
    { "zwj;", "\xE2\x80\x8D" },
    { "zwnj;", "\xE2\x80\x8C" }
};

static int cmp(const void *key, const void *value)
{
    return strncmp((const char *)key, *(const char *const *)value,
                   strlen(*(const char *const *)value));
}

static const char *get_named_entity(const char *name)
{
    const char *const *entity = (const char *const *)bsearch(name,
                                                             NAMED_ENTITIES, sizeof NAMED_ENTITIES / sizeof *NAMED_ENTITIES,
                                                             sizeof *NAMED_ENTITIES, cmp);
    
    return entity ? entity[1] : NULL;
}

static size_t putc_utf8(unsigned long cp, char *buffer)
{
    unsigned char *bytes = (unsigned char *)buffer;
    
    if(cp <= 0x007Ful)
    {
        bytes[0] = (unsigned char)cp;
        return 1;
    }
    
    if(cp <= 0x07FFul)
    {
        bytes[1] = (unsigned char)((2 << 6) | (cp & 0x3F));
        bytes[0] = (unsigned char)((6 << 5) | (cp >> 6));
        return 2;
    }
    
    if(cp <= 0xFFFFul)
    {
        bytes[2] = (unsigned char)(( 2 << 6) | ( cp       & 0x3F));
        bytes[1] = (unsigned char)(( 2 << 6) | ((cp >> 6) & 0x3F));
        bytes[0] = (unsigned char)((14 << 4) |  (cp >> 12));
        return 3;
    }
    
    if(cp <= 0x10FFFFul)
    {
        bytes[3] = (unsigned char)(( 2 << 6) | ( cp        & 0x3F));
        bytes[2] = (unsigned char)(( 2 << 6) | ((cp >>  6) & 0x3F));
        bytes[1] = (unsigned char)(( 2 << 6) | ((cp >> 12) & 0x3F));
        bytes[0] = (unsigned char)((30 << 3) |  (cp >> 18));
        return 4;
    }
    
    return 0;
}

static bool parse_entity(
                         const char *current, char **to, const char **from)
{
    const char *end = strchr(current, ';');
    if(!end) return 0;
    
    if(current[1] == '#')
    {
        char *tail = NULL;
        int errno_save = errno;
        bool hex = current[2] == 'x' || current[2] == 'X';
        
        errno = 0;
        unsigned long cp = strtoul(
                                   current + (hex ? 3 : 2), &tail, hex ? 16 : 10);
        
        bool fail = errno || tail != end || cp > UNICODE_MAX;
        errno = errno_save;
        if(fail) return 0;
        
        *to += putc_utf8(cp, *to);
        *from = end + 1;
        
        return 1;
    }
    else
    {
        const char *entity = get_named_entity(&current[1]);
        if(!entity) return 0;
        
        size_t len = strlen(entity);
        memcpy(*to, entity, len);
        
        *to += len;
        *from = end + 1;
        
        return 1;
    }
}

size_t decode_html_entities_utf8(char *dest, const char *src)
{
    if(!src) src = dest;
    
    char *to = dest;
    const char *from = src;
    
    for(const char *current; (current = strchr(from, '&'));)
    {
        memmove(to, from, (size_t)(current - from));
        to += current - from;
        
        if(parse_entity(current, &to, &from))
            continue;
        
        from = current;
        *to++ = *from++;
    }
    
    size_t remaining = strlen(from);
    
    memmove(to, from, remaining);
    to += remaining;
    *to = 0;
    
    return (size_t)(to - dest);
}

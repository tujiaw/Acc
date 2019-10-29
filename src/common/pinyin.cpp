#include "common/pinyin.h"
#include <stdio.h>   
#include "Util.h"
#include <list>
#include <regex>

//////////////////////////////////////////////////////////////////////////  
/// ����תƴ������ �ʺ�gb2312��utf8  
//////////////////////////////////////////////////////////////////////////  
#define HZ2PY_OUTPUT_BUF_ARRAY_SIZE 5120    //һ���ֶε�ƴ�������  
#define HZ2PY_MAX_PINYIN_SIZE 30            //һ���ֵ�ƴ�������  
#define HZ2PY_UTF8_CHECK_LENGTH 20          //����Ƿ�Ϊutf8����ʱ�������ַ�����  

bool safeAddToOutbuf(char* outbuf,int &iOutbuf, const char* pinyinValue,int iPinyinValue);
int is_utf8_string(const char *utf);
void pinyin_utf8(const char* inbuf,char* outbuf, bool initial=false,bool polyphone_support=false,bool m_blnFirstBig=false,bool m_blnAllBiG=false,bool m_LetterEnd=false, bool m_unknowSkip=true,bool m_filterPunc=true);

std::string getPinYin(const std::string& inbuf, bool initial/*=false*/,bool polyphone_support/*=false*/,bool m_blnFirstBig/*=false*/,bool m_blnAllBiG/*=false*/,bool m_LetterEnd/*=false*/, bool m_unknowSkip/*=true*/,bool m_filterPunc/*=true*/)
{
    std::string sourceUtf = inbuf;
    char outbuf[HZ2PY_OUTPUT_BUF_ARRAY_SIZE];
    memset(outbuf, '\0', sizeof(char)*HZ2PY_OUTPUT_BUF_ARRAY_SIZE);  

    if (!is_utf8_string(sourceUtf.c_str()))
        sourceUtf = Util::gbk2utf8(inbuf);

    pinyin_utf8(sourceUtf.c_str(), outbuf, initial, polyphone_support, m_blnFirstBig, m_blnAllBiG, m_LetterEnd, m_unknowSkip, m_filterPunc);
    return std::string(outbuf);
}

std::string getFullWithSeperator(const std::string& inbuf, const std::string& seperator)
{
	std::string pinyinFull = getPinYin(inbuf, false, true);
	std::regex patternSeparator(" ");
	pinyinFull = std::regex_replace(pinyinFull, patternSeparator, seperator);

	return pinyinFull;
}

std::string getInitialWithSeperator(const std::string& inbuf, const std::string& seperator)
{
	std::string pinyinInitial = getPinYin(inbuf, true, true);
	std::regex patternSeparator(" ");
	pinyinInitial = std::regex_replace(pinyinInitial, patternSeparator, seperator);

	return pinyinInitial;
}

std::string getFullAndInitialWithSeperator(const std::string& inbuf)
{
    std::string pinyinFull = getPinYin(inbuf, false, true);
    std::string pinyinInitial = getPinYin(inbuf, true, true);
    std::string ret = pinyinFull;
    if (pinyinFull != pinyinInitial)
        ret = pinyinFull + " " + pinyinInitial;

    return ret;
}

bool safeAddToOutbuf(char* outbuf,int &iOutbuf, const char* pinyinValue,int iPinyinValue) {  
    int iOutbufWord=1,iPinyinValueWord=1,m;  
    for (m = 0; m<iOutbuf; ++m) { //ͳ���Ѿ�ת���Ĵ���  
        if (outbuf[m]==' '){  
            iOutbufWord++;  
        }  
    }  
    for (m = 0; m<iPinyinValue; ++m) {    //ͳ�ƶ�������Ŀ  
        if (pinyinValue[m]=='|'){  
            iPinyinValueWord++;  
        }  
    }  
    bool flag=false;    //�Ƿ񳬳���Χ  
    int iNewOutbuf=iOutbufWord*iPinyinValue+iPinyinValueWord*iOutbuf+iOutbufWord+iPinyinValueWord-1-iOutbufWord*iPinyinValueWord;//����Ϊɶ  
    if (iNewOutbuf<HZ2PY_OUTPUT_BUF_ARRAY_SIZE){  
        int totalWord=iOutbufWord*iPinyinValueWord;  
        //�ռ�����  
        char ** tempOutbuf=new char *[totalWord];  
        if(!tempOutbuf){  
            fprintf(stdout,"FATAL ERROR: out of memory (failed to malloc %d bytes)\n",totalWord);  
            return false;  
        }  
        for (m = 0; m<totalWord; ++m) {
            tempOutbuf[m]=new char[HZ2PY_OUTPUT_BUF_ARRAY_SIZE];  
            if(!tempOutbuf[m]){  
                fprintf(stdout,"FATAL ERROR: out of memory (failed to malloc %d bytes)\n",HZ2PY_OUTPUT_BUF_ARRAY_SIZE);  
                while(m>0){  
                    delete [] tempOutbuf[m-1];  
                    m--;  
                }  
                delete[] tempOutbuf;  
                return false;  
            }  
        }  
        char *tmp1=outbuf;  
        int n=0,i,j,k;  
        while (*tmp1!='\0'||n==0)   //ע�⣬�������ʼ���ʼ���n==0  
        {  
            n+=iPinyinValueWord;  
            i=0;  
            while (*tmp1!=' '&&*tmp1!='\0')  
            {  
                for (m = n - iPinyinValueWord; m<n; ++m)
                {  
                    tempOutbuf[m][i]=*tmp1;  
                }  
                i++;  
                tmp1++;  
            }  
            if (*tmp1==' ')  
            {  
                tmp1++;  
            }  
            k=0;  
            for (m = n - iPinyinValueWord; m<n; ++m)
            {  
                j=i;  
                while(k<iPinyinValue&&pinyinValue[k]!='|'&&pinyinValue[k]!='\0'){  
                    tempOutbuf[m][j]=pinyinValue[k];  
                    j++;  
                    k++;  
                }  
                if (pinyinValue[k]=='|')  
                {  
                    k++;  
                }  
                tempOutbuf[m][j]='\0';  
            }  
        }  
  
        //�����еĶ��������ϵ�outbuf��  
        outbuf[0]='\0';  
        for (m = 0; m<totalWord; ++m)
        {  
            outbuf=strcat(outbuf,tempOutbuf[m]);  
  
            if (m!=totalWord-1)  
            {  
                outbuf=strcat(outbuf," ");  
            }  
        }  
        iOutbuf=iNewOutbuf;  
        //ɾ��������Ŀռ�  
        for (m = 0; m<totalWord; ++m)
        {  
            delete[] tempOutbuf[m];  
        }  
		delete[] tempOutbuf;
        flag=true;  
    }  
    return flag;  
}  
  
//////////////////////////////////////////////////////////////////////////  
//�ж�һ���ַ����Ƿ�Ϊutf8����  
//////////////////////////////////////////////////////////////////////////  
const char *_pinyin_table_[20902] = {  
    "yi", "ding|zheng", "yu", "qi", "shang", "xia", "myeon", "wan|mo", "zhang", "san", "shang", \
    "xia", "qi", "bu|dun|fou|fu", "yu", "mian", "gai", "chou", "chou", "zhuan", "qie|ju", \
    "pi", "shi", "shi", "qiu", "bing", "ye", "cong", "dong", "si", "cheng|sheng|zheng", \
    "diu", "qiu", "liang", "diu", "you", "liang", "yan", "ban|bang|bing", "sang", "shu", \
    "jiu", "gan|ge", "ya", "pan", "zhong", "ji", "jie", "feng", "guan|kuang", "chuan|guan", \
    "chan", "lin", "zhuo", "zhu", "ha", "wan", "dan", "wei", "zhu", "dan|jing", \
    "li", "ju", "pianpang|pie|yi", "fu", "yi", "ai|yi", "ai|nai", "wu", "jiu", "jiu", \
    "zhe|tuo", "me|mo|yao|ma", "xi|yi", "ho", "zhi", "wu", "zha|zuo", "hu", "fa", "le|yue", \
    "pan|yin", "ping", "pang", "jiao|qiao", "hu", "guai", "cheng|sheng", "cheng|sheng", "jue|yi", "yin", \
    "wan|ya", "mie|nie", "jiu", "qi", "ye|yi", "xi", "xiang", "gai", "jiu", "hal", \
    "hol", "shu", "dou|dul", "shi", "ji", "keg|nang", "kal", "keol", "tol", "mol", \
    "ol", "mai", "luan", "cal", "ru", "xue", "yan", "phoi", "sal|sha", "na", \
    "gan|qian", "sol", "eol", "cui", "ceor", "qian|gan", "zhi|luan", "gui", "gan", "luan", \
    "lin", "yi", "jue", "le|liao", "ma", "yu|zhu", "zheng", "shi|zhi", "shi|zi", "er", \
    "chu", "xu|yu", "kui|yu", "xu|yu", "yun", "hu", "qi", "wu", "jing", "si", \
    "sui", "gen|xuan", "geng|gen", "e|ya", "xie|suo", "e|ya", "zhai|qi", "e|ya", "ji|qi", "wen", \
    "wang|wu", "gang|geng|kang", "da", "jiao", "hai|jie", "yi", "chan", "heng|peng|xiang", "mu", "ye", \
    "xiang", "jing", "ting", "liang", "xiang", "jing", "ye", "qin|qing", "bo", "you", \
    "xie", "chan|dan|zhan", "lian", "duo", "men|wei", "ren", "ren", "ji", "ra", "wang|wu", \
    "yi", "shen|shi", "ren", "le|li", "ding", "ze", "fu|jin|nu", "pu", "chou|qiu", "ba", \
    "zhang", "jin", "ge|jie", "bing", "reng", "cong", "fo", "san|jin|tao", "lun", "e|bing", \
    "cang|chuang", "zai|zi", "shi", "ta|tuo", "zhang", "fu", "xian", "xian", "tuo|duo|cha", "hong", \
    "tong", "ren", "qian", "gan|han", "yi|ge", "bo", "dai", "lian|ling", "si|yi", "chao", \
    "chang|zhang", "sa", "shang", "yi", "mu", "men", "ren", "fan", "chao|miao", "ang|yang", \
    "qian|jing", "zhong", "bi|pi", "wo", "wu", "jian|mou", "jia|jie", "yao|fo", "feng", "cang|chuang", \
    "lin|ren", "wang", "bin|fen", "di", "fang|pang", "zhong", "qi", "pei", "xu|yu", "diao", \
    "dun", "wu|wen", "yi", "lin|xin", "gang|kang", "yi", "fan|ji", "ai", "wu", "ji|qi", \
    "fu", "fa", "xiu|xu", "jin|yin", "pi", "dan|shen", "fu", "nu|tang", "yin|zhong", "you", \
    "huo", "hui|kuai", "yu", "cui|zu", "yun", "san", "wei", "chuan|zhuan", "che|ju", "ya", \
    "qian|xian", "shang", "chang|cheng|zheng", "lun", "cang|chen", "xun", "xin", "e|gui|wei", "zhu", "chi|che", \
    "xuan|xian", "nu", "bo|bai", "gu", "ni", "ni", "xie", "ban|pan", "xu", "ling", \
    "zhou", "shen", "qu|zu", "ci|si", "peng|beng", "si|shi", "jia|ga|qie", "pi", "zhi|yi", "shi|si", \
    "yi|si|chi", "zheng", "dian|tian", "gan|han", "mai", "dan|tan|yan", "zhu", "bu", "qia|qu", "bi", \
    "zhao|shao", "ci", "li|wei", "di", "zhu", "zuo", "you", "yang", "ben|cui|ti", "zhan|dian|chan", \
    "he", "bi", "tuo|yi", "she", "tu|xu|yu", "yi|die", "fo|fu", "zuo", "gou|kou", "ning", \
    "tong", "ni", "xian", "qu", "yong", "wa", "qian", "shi", "ka", "bao", \
    "pei", "huai|hui", "ge|he", "lao|liao", "xiang", "ge|e", "yang", "bai|mo", "fa", "ming", \
    "jia", "er|nai", "bing", "ji", "hen|heng", "huo", "gui", "quan", "diao|tiao", "jiao|xiao", \
    "ci", "yi", "shi", "xing", "shen", "tuo", "kan", "zhi", "hai|gai", "lai", \
    "yi", "chi", "e|hua|kua|wu", "guang|gong", "li|lie", "yin", "shi", "mi", "zhou|zhu", "xu", \
    "you", "an", "lu", "mao|mou", "er", "lun", "tong|dong", "cha", "chi", "xun", \
    "gong", "zhou", "yi", "ru", "cun|jian", "xia", "si", "dai", "lv", "ta", \
    "jiao|yao", "zhen", "ce|zhai", "jiao|qiao", "kuai", "chai", "ning", "nong", "jin", "wu", \
    "hou", "jiong", "cheng|ting", "chen|zhen", "zuo", "chou", "qin", "lv", "ju", "dou|shu", \
    "ting", "shen", "tuo|tui", "bo", "man|nan", "xiao", "bian|pian", "tui", "yu", "xi", \
    "chuo|cu", "e", "qiu", "xu|shu", "guang", "ku", "wu", "dun|jun|shun", "yi", "fu", \
    "liang|lang", "zu", "qiao|xiao", "li", "yong", "hun", "jing|ying", "xian|qian", "san", "pei", \
    "su", "fu", "xi", "li", "mian|fu", "ping", "bao", "yu|shu", "si|qi", "xia", \
    "shen|xin", "xiu", "yu", "di", "che|ju", "chou|dao", "zhi", "yan", "lia|liang", "li", \
    "lai", "si", "jian", "xiu", "fu", "huo", "ju", "xiao", "pai", "jian", \
    "biao", "shu|chu|ti", "fei", "beng|feng", "ya", "an|yan", "bei", "yu", "xin", "bei|bi|pi", \
    "hu|chi", "chang|cheng", "zhi", "bing", "jiu", "yao", "zu|cui", "lia|liang", "wan", "lai|lie", \
    "cang|chuang", "zong", "ge", "guan", "bei|pei", "tian", "shu", "shu", "men", "dao", \
    "dan|tan", "jue", "chui|zhui", "xing", "peng|ping", "tang|chang", "hou", "ji|yi", "qi", "diao|ti|zhou", \
    "gan", "jing|liang", "jie", "sui", "chang", "jie|qie", "fang", "zhi", "kong", "juan", \
    "zong", "ju", "qian|qing", "ni|nie", "lun", "zhuo", "wei|wo", "luo", "song", "ling|leng", \
    "hun", "dong", "zi", "ben", "wu", "ju", "nai", "cai", "jian", "zhai", \
    "ya|ye", "zhi", "sha", "qing", "qie", "ying", "chen|cheng", "qian|jian", "yan", "ruan|ru", \
    "zhong|tong", "chun", "jia|jie|xia", "jie|ji", "wei", "yu", "bing", "re|ruo", "ti", "wei", \
    "pian", "yan", "feng", "tang|dang", "wo", "e", "xie", "che", "sheng", "kan", \
    "di", "zuo", "cha", "ting", "bei", "ye|xie", "huang", "yao", "zhan", "chou|qiao", \
    "yan|an", "you", "jian", "xu", "zha", "ci", "fu", "fu|bi", "zhi", "cong|zong", \
    "mian", "ji", "yi", "xie", "xun", "si|cai", "duan", "ce|ze|zhai", "zhen", "ou", \
    "tou", "tou", "bei", "zan|za", "lou|lv", "jie|qie", "e|gui|wei", "fen", "chang", "kui|gui", \
    "sou", "si|zhi", "su", "xia", "fu", "yuan", "rong", "li", "nu", "yun", \
    "gou|jiang", "ma", "bang|beng|pang|peng", "dian", "tang", "hao", "jie", "xi", "shan", "qian|jian", \
    "que|jue", "cang|chen", "chu", "san", "bei", "xiao", "rong|yong", "yao", "ta|tan", "suo", \
    "yang", "fa", "bing", "jia|jie", "dai", "zai", "tang", "gu", "bin", "chu", \
    "nuo", "can|san", "lei", "cui", "yong|chong", "cao|zao", "zong", "beng|peng", "song|shuang", "ao", \
    "chuan|zhuan", "yu", "zhai", "qi|zu|cou", "shang", "chuang", "jing", "chi", "sha", "han", \
    "zhang", "qing", "yan|yin", "di", "su|xie", "lou|lv", "bei", "biao|piao", "jin", "lian", \
    "liao|lu", "man", "qian", "xian", "tan|lan", "ying", "dong", "zhuan|zun", "xiang", "shan", \
    "jiao|qiao", "jiong", "tui", "cuan|zun", "bu|pu", "xi", "lao", "chang", "guang", "lao|liao", \
    "qi", "deng|cheng|teng", "chan|zhuan", "e|gui|wei", "ji", "bo", "hui", "chun|chuan", "tie|jian", "dan|chan", \
    "jiao|yao", "jiu", "ceng|seng", "fen", "xian", "ju|yu", "e", "jiao", "jian|zen", "tong|zhuang", \
    "lin", "bo", "gu", "xian", "su", "xian", "jiang", "min", "ye", "jin", \
    "jia|qia", "qiao", "pi", "feng", "zhou", "ai", "sai", "yi", "jun", "nong", \
    "tan|chan|shan", "yi", "dang", "jing", "xuan", "kuai", "jian", "chu", "dan|shan", "jiao", \
    "sha", "zai", "can", "bin", "an", "ru", "tai", "chou|dao", "chai", "lan", \
    "yi|ni", "jin", "qian", "meng", "wu", "ning", "qiong", "ni", "chang", "la|lie", \
    "lei", "lv", "kuang", "bao", "yu|di", "biao", "zan", "zhi", "si", "you", \
    "hao", "qing|jing", "qin|chen", "li", "teng", "wei", "long", "chu", "chan", "rang|xiang", \
    "tiao|shu", "xie|hui", "li", "luo", "zan", "nuo", "chang|tang", "yan", "lei|luo", "nang", \
    "er|ren", "wu", "yuan|yun", "zan", "yuan", "kuang|xiong", "chong", "zhao", "xiong", "xian", \
    "guang", "dui|yue|rui", "ke", "dui|yue|rui", "mian|wan|wen", "chan|tu", "chang|zhang", "er|ni", "dui|duo|rui", "er|ni", \
    "jin", "chan|tu", "si", "yan", "yan", "shi", "shike", "dang", "qianke", "dou", \
    "gongfen", "haoke", "shen", "dou", "baike", "jing", "gongli", "huang", "ru", "wu|wang", \
    "nei|na", "quan", "liang", "yu|shu", "ba", "gong", "liu|lu", "xi", "han|jie", "lan", \
    "gong", "tian", "guan|wan", "xin|xing", "bing", "qi|ji", "ju", "dian|tian", "zi|ci", "ppun", \
    "yang", "jian", "shou", "ji", "yi", "ji", "chan", "jiong|tong", "mao", "dan|ran", \
    "na|nei|rui", "yan", "mao", "gang", "dan|ran", "ce|zha", "jiong", "ce|zha", "zai", "gua", \
    "jiong", "mao|mo", "zhou", "mao|mo", "gou", "xu", "mian", "tu", "rong", "you|yin", \
    "xie", "hem|kan", "jun", "nong", "yi", "mi|shen", "shi", "guan", "meng", "zhong", \
    "ju|zui", "yuan", "mian|ming", "kou", "lem|min", "fu", "xie", "mi", "liang", "dong", \
    "tai", "gang", "feng|ping", "bing|ning", "hu", "chong", "jue|que|xue", "ya|hu", "kuang", "ye", \
    "leng|ling", "pan", "fa|fu", "min", "dong", "sheng|xian", "lie", "qia", "jian", "cheng|jing", \
    "sou", "mei", "tu", "qi|qian", "gu", "zhun", "song", "jing|cheng", "liang", "qing", \
    "diao", "ling", "dong", "gan", "jian", "yin", "cou", "ai", "li", "cang", \
    "ming", "zhun", "cui", "si", "duo", "jin", "lin", "lin", "ning", "xi", \
    "du|dou", "ji", "fan", "fan", "fan", "feng", "ju", "chu", "zheng", "feng", \
    "mu", "zhi", "fu", "feng", "ping", "feng", "kai", "huang", "kai", "gan", \
    "deng", "ping", "qian", "xiong", "kuai", "tu", "ao|wa", "chu", "ji", "dang", \
    "han", "han", "zao|zuo", "dao|diao", "diao", "li", "ren", "ren", "chuang", "ban|fen", \
    "qi|qie", "yi", "ji", "kan", "qian", "cun", "chu", "wen", "ji", "dan", \
    "xing", "hua|huai", "wan", "jue", "li", "yue", "li|lie", "liu", "ze", "gang", \
    "chuang", "fu", "chu", "qu", "diao", "shan", "min", "ling", "zhong", "pan", \
    "bie", "jie", "jie", "pao|bao", "li", "shan", "bie", "chan", "jing", "gua", \
    "geng", "dao", "chuang", "kui", "ku|kuo", "duo", "er", "zhi", "shua", "quan|xuan", \
    "sha|cha", "ci|qi", "ke|kei", "jie", "gui", "ci", "gui", "ai|kai", "duo", "ji", \
    "ti", "jing", "dou", "luo", "ze", "yuan", "cuo", "xiao|xue", "ke|kei", "la", \
    "jian|qian", "sha|cha", "chuang", "gua", "jian", "cuo", "li", "ti", "fei", "pou", \
    "chan", "qi", "chuang", "zi", "gang", "wan", "bo|bao|pu", "ji", "duo|chi", "qing|lue", \
    "yan|shan", "zhuo|du", "jian", "ji", "bo|bao", "yan", "ju", "huo", "sheng", "jian", \
    "du|duo", "duan|tuan|zhi", "wu", "gua", "fu|pi", "sheng", "jian", "ge", "da|zha", "ai|kai", \
    "chuang|qiang", "chuan", "chan", "zhuan|tuan", "lu|jiu", "li", "peng", "shan", "biao|piao", "kou", \
    "jiao|chao", "gua", "qiao", "jue", "hua", "zha", "zhu|zhuo", "lian", "ju", "pi", \
    "liu", "gui", "chao|jiao", "gui", "jian", "jian", "tang|tong", "hua|huo", "ji", "jian", \
    "yi", "jian", "zhi", "chan", "zuan", "mi|mo", "li", "zhu", "li", "ya", \
    "quan", "ban", "gong", "jia", "wu", "mai", "lie", "jin|jing", "keng", "xie|lie", \
    "zhi", "dong", "chu|zhu", "nu", "jie", "qu", "shao", "yi", "zhu", "miao|mo", \
    "li", "jin|jing", "lao|liao", "lao", "juan", "kou", "gu|yang", "wa", "xiao", "mou", \
    "kuang", "jie", "lie", "he|kai", "shi", "ke|kei", "jin|jing", "gao", "bo", "min", \
    "chi", "lang", "yong", "yong", "mian", "ke|kei", "xun", "juan", "qing", "lu", \
    "bu", "meng", "chi|lai", "le|lei", "kai", "mian", "dong", "mao|xu", "xu", "kan", \
    "mao|wu", "yi", "xun", "weng|yang", "sheng", "lao|liao", "bo|mu", "lu", "piao", "shi", \
    "ji", "qi|qin", "jiang|qiang", "jiao|chao", "quan", "xiang", "yi", "qiao|jue", "fan", "juan", \
    "tong|dong", "ju", "dan", "xie", "mai", "xun", "xun", "lv", "li", "che", \
    "rang|xiang", "quan", "bao|pianpang", "di|shao|zhuo", "jun|yun", "jiu", "bao", "gou", "mo|wu", "jun|yun", \
    "mangmi", "bi", "gai", "gai", "bao|fu|pao", "cong", "yi", "xiong", "peng", "ju", \
    "tao|yao", "ge", "pu", "e", "pao", "fu", "gong", "da", "jiu", "gong", \
    "bi|pin", "hua|huo", "bei", "nao", "shi|chi", "fang|pianpang", "jiu", "yi", "za", "jiang", \
    "kang", "jiang", "kuang|wang", "hu", "xia", "qu", "fan", "gui", "qie", "zang|cang", \
    "kuang", "fei|fen", "hu", "yu", "gui", "kui|gui", "hui", "dan", "kui|gui", "lian", \
    "lian", "suan", "du", "jiu", "qu|jue", "xi", "pi", "ou|qu", "yi", "an|e|ke", \
    "yan", "bian", "ni|te", "ou|qu", "shi", "xun", "qian", "nian", "sa", "cu|zu", \
    "sheng", "wu", "hui", "ban|pan", "shi", "xi", "wan", "hua", "xie", "wan", \
    "bei|bi|pi", "zu|cu", "zhuo", "xie", "shan|dan|chan", "mai", "nan|na", "chan|dan|shan", "chi|ji", "bo", \
    "shuai|lv", "bu|bo", "kuang|guan", "bian|pan", "bu|ji", "tie|zhan", "ka|qia", "lei|lu|lv", "you", "lu", \
    "xi", "gua", "wo", "xie", "dan|jie", "jie|ran", "wei", "ang|yang", "qiong", "zhi", \
    "mao", "yi|yin", "wei", "shao", "ji", "jiao|que", "kun|luan", "chi", "juan|quan", "xie", \
    "xu|su", "jin", "jiao|que|xi", "kui", "ji", "e", "qing", "xi", "san", "an|chang|yan", \
    "yan", "e", "ting", "li", "zhai|zhe", "an|han", "li", "ya", "ya", "ya|yan", \
    "she", "di", "zha|zhai", "long|pang", "ting|a", "qie", "ya", "zhi|shi", "ce|si", "mang", \
    "ti", "chan|li", "she", "hou", "ting", "zui", "cuo|ji", "fei", "yuan", "ce|ci|si", \
    "yuan", "xiang", "yan", "li", "jue", "xia|sha", "dian", "chu", "jiu", "jin|qin", \
    "ao", "gui", "yan|ya", "si", "li", "chang", "qian|lan", "lai|li", "yan", "yan", \
    "yuan", "si", "hong|gong", "miao|lin|mian", "rou|qiu", "qu", "qu", "keum", "lei", "du", \
    "xian", "zhuan|hui", "san", "can|shen|cen", "can|shen|cen|san", "can|shen|cen|san", "can|shen|cen|san", "ai|yi", "dai", "you", \
    "cha|chai", "ji", "you", "shuang", "fan", "shou", "guai|jue", "ba", "fa", "ruo", \
    "shi|li", "shu", "zhuo|jue|yi|li", "qu", "dao|shou", "bian", "xu", "jia|xia", "pan", "sou|xiao", \
    "ji", "yu|wei", "sou|xiao", "die", "rui", "cong", "kou", "gu|ku", "ju|gou", "ling", \
    "gua", "dao|tao", "kou", "zhi", "jiao", "zhao|shao", "ba|pa", "ding", "ge|ke", "si|tai|yi", \
    "chi", "shi", "you", "qiu", "po", "ye|xie", "hao|xiao", "ci|si", "tan|yi|you", "chi", \
    "le|li", "diao", "ji", "dug", "hong", "mie", "xu|yu", "mang", "chi|qi", "ge", \
    "song|xuan", "yao", "ji|zi", "he|ge", "ji", "diao", "cun|duo|dou", "tong", "ming", "hou", \
    "li", "tu", "xiang", "zha", "xia|he", "ye", "lv", "ya|a", "ma", "ou", \
    "huo", "yi|xi", "jun", "chou", "lin", "tian|tun", "jin|yin", "fei", "bi|pi", "qin", \
    "qin", "jie|ge", "bu|pou", "fou|pi", "ba|pa", "dun|tun", "fen|pen", "hua|e", "han", "ting|yin", \
    "hang|keng", "shun", "qi", "hong", "zhi|zi", "yin|shen", "wu|yu", "tun|wu", "chao|miao", "na|ne", \
    "jue|xue|chuo", "xi", "chui", "dou|ru", "wen", "hou", "hong|ou|hou", "wu|ya|yu", "gao", "xia|ya", \
    "jun", "lv", "ai|e", "ge", "wen|mei", "dai|ai", "qi", "cheng", "wu", "gao|gu|ju", \
    "fu", "jiao", "yun", "chi", "sheng", "na|ne", "tun|tian", "fu|mu", "yi", "tai|dai", \
    "ou|xu", "li", "bei|bai", "yuan|yun", "wa|wo|gua|guo|he|wai", "wen|hua|qi", "cheng|qiang", "wu", "e|ai", "shi", \
    "juan", "pen", "min|wen", "ne|ni", "mu|mou", "ling", "ran", "you", "di", "zhou", \
    "shi", "zhou", "tie|che", "xi|chi", "yi", "zhi|qi", "ping", "ci|zi", "gua|gu", "ci|zi", \
    "mei|wei", "hou|gou|xu", "he|ke", "na|nao|nu", "xia|ga", "pei", "chi|yi", "hao|xiao", "shen", "he|hu|xiao|xu", \
    "ming", "dan|da|ya", "qu|ka", "ju|zui", "gan|han|xian", "za", "tuo", "duo", "pou", "pao", \
    "bi|bie", "fu", "yang", "he|huo", "za|zha|ze", "he|huo|hu", "tai|hai", "gao|jiu", "yong", "fu", \
    "da", "zhou", "wa", "ka|nong", "gu", "ka|ga", "zo|zuo", "bu", "long", "dong", \
    "ning", "ta|tuo", "si", "xian", "huo", "qi", "er", "e", "gong|guang", "zha", \
    "die|xi", "xi|yi", "lei|lie", "zi", "mie", "mai|mie", "zhi", "jiao|yao", "ji|xi|qia", "ru|zhou|zhu", \
    "ge|luo|lo|ka", "xun|shu", "zan|za", "xiao", "ke|hai", "hai|hui", "kua", "shi|huai", "tiao|tao", "xian", \
    "an|e", "xuan", "xiao|xiu|xu", "guo|wai", "yan|ye", "lao", "yi", "ai", "pin", "shen", \
    "tong", "hong", "hong|xiong", "duo|chi", "gui|hua|wa", "ha|he", "zai", "you", "di|mi", "gu|pai", \
    "xiang", "ai", "gen|hen", "kuang|qiang", "e|ya", "da", "xiao", "bi", "hui|yue", "nian", \
    "hua", "xing", "guai|kuai|kuo|wei", "duo", "ppun", "ji|zhai|jie", "nang|nong", "mou", "yo", "hao", \
    "yuan|yun", "long|ka", "tou|pou", "mang", "ge", "o|e", "chi|xia", "sao|shao|xiao", "li|mai", "na|ne", \
    "zu", "he", "ku", "xiao|xue", "xian", "lao", "bo|po", "zhe", "zha", "liang|lang", \
    "ba", "mie", "lv|lie", "sui", "fu", "bu|fu", "han", "heng", "geng|ying", "shuo|yue", \
    "jia|ge", "you", "yan", "gu", "gu", "bei|bai", "han", "shua|suo", "chun|zhen", "yi", \
    "ai", "jia|qian", "tu", "dan|xian|yan", "wan", "li", "xi|xie", "tang", "shi|zuo", "qiu", \
    "che", "wu", "zao", "ya|e", "dou", "qi", "di", "qin", "mai|ma", "mas", \
    "gong|hong", "teo", "keos", "chao|lao|xiao", "liang", "suo", "zao", "huan", "lang", "sha", \
    "ji|jie", "zo", "wo|wei", "beng|feng", "jin|yin", "hu|xia", "qi", "shou|shu", "wei", "shua", \
    "chang", "er|wa", "li", "qiang", "an", "ze|zuo|jie", "yo|yu", "nian|dian", "yu", "tian", \
    "lai", "qie|sha", "xi", "tuo", "hu", "ai", "zhou|zhao|tiao", "nou|gou", "ken", "zhou|zhuo", \
    "zhuo|zhao", "shang", "di|shi", "heng|e", "lin|lan", "a", "cai|xiao", "qiang|xiang", "tun|zhun|dui", "wu", \
    "wen", "cui|qi", "jie|die|ti|sha", "gu", "qi", "qi", "tao", "dan", "dan", "yue|wa|ye", \
    "ci|zi", "bi|tu", "cui", "chuo|chuai", "he", "ya|e", "qi", "zhe", "fei|pai|pei", "liang", \
    "xian", "pi", "sha", "la", "ze", "qing|ying", "gua", "pa", "ze|shi", "se", \
    "zhuan", "nie|yao", "guo", "luo", "ngam", "di", "quan|jue", "chan|tan|tuo", "bo", "ding", \
    "lang", "chi|xiao", "geu", "tang", "chi|di", "ti", "an", "jiu", "dan", "ka|ke", \
    "yu|yong", "wei", "nan", "shan", "yu", "zhe", "la", "jie|xie", "hou", "han|jian|kan", \
    "die|zha", "zhou", "chai", "wai", "nuo|re", "yu|xu|guo|huo", "yin", "zan|za", "yao", "wo|o", \
    "mian", "hu", "yun", "chuan", "hui|zhou", "huan", "yuan|huan|xuan|he", "chi|xi", "he|ye", "ji", \
    "huai|kui", "zhong|chong|chuang", "wei", "che|sha", "xu", "huang", "duo|zha", "yan|nie", "xuan", "liang", \
    "yu", "sang", "chi|kai", "qiao|jiao", "yan", "dan|shan|chan", "ben|pen", "can|qi|sun", "li", "yo", \
    "zha|cha", "wei", "miao", "ying", "fen|pen", "phos", "kui", "bei", "yu", "gib", \
    "lou", "ku", "zao|qiao", "gu|hu", "ti", "yao", "he|xiao|hu", "sha|a", "xiu|xu", "cheng|qiang", \
    "se", "yong", "su", "gong|hong", "xie", "yi|ai", "shuo|suo", "ma", "cha", "hai", \
    "he|ke|xia", "da|ta", "sang", "chen|tian", "ru", "sou|su", "gu|wa", "ji", "pang|beng", "wu", \
    "qian|xian|qie", "shi", "ge", "zi", "jie|jue", "lao|lu", "weng", "wa", "si", "chi", \
    "hao", "suo", "jia|lun", "hai|hei", "suo", "qin", "nie|zhe", "he", "cis", "sai", \
    "en|ng", "go", "na", "dia", "ai", "qiang", "tong", "bi", "ao", "ao", \
    "lian", "zui|sui", "zhe|zhu", "mo", "shu|shuo|sou", "sou", "tan", "di|zhe", "qi|za|zu", "jiao", \
    "chong", "jiao|dao", "kai|ge", "tan", "can|shan", "cao", "jia", "ai", "xiao", "piao", \
    "lou", "ga", "jia|gu", "xiao|jiao", "hu", "hui", "guo", "ou|xu", "xian", "ze", \
    "chang", "xu|shi", "po", "de|dei", "ma", "ma", "hu", "le|lei", "du", "ga", \
    "tang", "ge|ye", "beng", "ying", "sai", "jiao", "mi", "chi|xiao", "hua", "mai", \
    "ran", "zuo|zhuai|chuai", "peng", "chao|lao|xiao", "chi|xiao", "ji", "zhu", "chao|zhao", "huai|kui", "zui", \
    "xiao", "si", "hao", "mu|fu", "liao", "qiao", "xi", "xu|chu", "tan|chan", "tan|dan", \
    "hei|mo|mu", "xun", "e|wu", "zun", "fan|bo", "chi|kai", "hui", "can", "chuang", "cu|za", \
    "dan", "yu", "tun|kuo", "ceng|cheng", "jiao|jiu", "sha|ye|yi", "xi", "qi", "hao", "lian", \
    "xu|shi", "deng", "hui", "yin", "pu", "jue", "qin", "xun", "nie|yao", "lu", \
    "si", "yan", "ying", "da", "zhan|dan", "ao|yu", "zhuo|zhou", "jin", "nang|nong", "hui|yue", \
    "xie", "qi", "e", "zao", "ai|yi", "shi", "jiao|qiao|chi", "yuan", "ai", "yong", \
    "xue|jue", "kuai|guai", "yu", "fen|pen", "dao", "ga|ge", "xin|hen", "dun", "dang", "xin", \
    "sai", "pi", "pi", "yin", "zui", "ning", "di", "lan|han", "ta", "huo|wo|o", \
    "ru", "hao", "he|xia", "yan|ye", "duo", "xiu|pi", "zhou|chou", "ji|zhai|jie", "jin", "hao", \
    "ti", "chang", "xun", "me", "cha|ca", "ti|zhi", "lu", "hui", "bao|bo|pao", "you", \
    "nie|yao", "yin", "yo|hu", "mo|mei|me", "hong", "zhe", "li", "liu", "xie|hai", "nang", \
    "xiao|ao", "mo", "yan", "li", "lu", "long", "po|mo", "dan", "chen", "pin", \
    "pi", "xiang", "huo|xue", "me", "xi", "duo", "ku", "yan", "chan", "ying", \
    "rang", "dian|di", "la", "ta", "xiao|ao", "jiao|jue", "chuo", "huan", "geo|huo", "zhuan", \
    "nie|zhe", "xiao|ao", "za|zha|ca", "li", "chan", "chai", "li", "yi", "luo", "nang", \
    "zan|za|can", "su", "heui", "zeng|zen", "jian", "yan|za|nie", "zhu", "lan", "nie", "nang", \
    "ram", "luo|lo", "guo|wei", "hui", "yin", "qiu", "si", "nin", "nan|jian", "hui", \
    "xin", "yin", "nan|nie", "qiu|tuan", "tuan", "tun|dun", "kang", "yuan", "jiong", "pian", \
    "yun", "cong|chuang", "hu", "hui", "wan|yuan", "e", "guo", "kun", "chuang|cong", "tong|wei", \
    "tu", "wei", "lun", "guo", "qun", "ri|shi", "ling", "gu", "guo", "tai", \
    "guo", "tu", "you", "guo", "yin", "hun|huan", "pu", "yu", "han", "yuan", \
    "lun", "quan|juan", "yu", "qing", "guo", "chuan|chui", "wei", "yuan", "quan|juan", "ku", \
    "pu", "yuan", "yuan", "ya", "tuan", "tu", "tu", "chan|tuan", "lue", "hui", \
    "yi", "huan|yuan", "luan", "luan", "cha|du|tu", "ya", "tu", "ting", "ku|sheng", "pu", \
    "lu", "kuai", "ju|ya", "zai", "xu|wei", "ge|yi", "yu|zhun", "wu", "gui", "pi", \
    "yi", "di|de", "qian|su", "qian|su", "chou|huai|quan|zhen", "zhuo", "dang", "qia", "xia", "shan", \
    "kuang", "chang|dang|shang", "qi|yin", "nie", "mo", "ji|jie", "jia", "zhi", "zhi", "ban", \
    "xun", "yi", "qin", "mei|fen", "jun|yun", "keng|rong", "tun|dun", "fang", "fen|ben", "ben", \
    "tan", "kan", "huai|pei|pi", "zuo", "kang|keng", "bi", "jing|xing", "lan|di", "jing", "ji", \
    "kuai|yue", "di|chi", "jing|xing", "jian", "dan|tan", "li", "ba", "wu", "fen", "zhui", \
    "po", "ban|pan", "tang", "kun", "ju|qu", "tan", "zhi|zhuo", "tuo|yi", "gan", "ping", \
    "dian|zhen", "gua", "ni", "tai", "huai|pi", "jiong|shang", "yang", "fo", "ao|you", "lu", \
    "qiu", "mei|mu", "ke", "gou", "xue", "ba", "di|chi", "che", "ling", "zhu", \
    "fu", "hu", "zhi", "chui|zhui", "la", "long", "long", "lu", "ao", "dai|tae", \
    "pao", "min", "xing", "dong|tong", "ji", "he", "lv", "ci", "chi", "lei", \
    "gai", "yin", "hou", "dui", "zhao", "fu", "guang", "yao", "duo", "duo", \
    "gui", "cha", "yang", "ken|yin", "fa", "gou", "yuan", "die", "xie", "ken|yin", \
    "jiong|shang", "shou", "e|sheng", "bing", "dian", "hong", "e|wu|ya", "kua", "da", "ka", \
    "dang", "kai", "hang", "nao", "an", "xing", "xian", "yuan|huan", "bang", "fu|pou", \
    "ba|bei", "yi", "yin", "han|an", "xu", "chui|zhui", "cen|qin", "geng", "ai|zhi", "feng|beng", \
    "fang|di", "jue|que", "yong", "jun", "jia|xia", "di", "mai|man", "lang", "juan", "cheng", \
    "yan|shan", "qin|jin", "zhe", "lie", "lie", "pu|bu", "cheng", "hua", "bu", "shi", \
    "xun", "guo|wo", "jiong", "ye", "dian|nian|nie", "di", "yu", "bu", "ya|e", "juan|quan", \
    "sui|su", "bei|bi|pi", "qing|zheng", "wan", "ju", "lun", "zheng|cheng", "kong", "shang|chong", "dong", \
    "dai", "tan", "an|yan", "cai", "chu|tou", "bang|beng", "xian|kan", "zhi", "duo", "yi|shi", \
    "zhi", "yi", "pei|pi|pou", "ji", "zhun|dui", "qi", "sao", "ju", "ni|ban", "ku", \
    "ke", "tang", "kun", "ni", "jian", "dui|zui", "jin|qin", "gang", "yu", "e|ya", \
    "peng|beng", "gu", "tu", "leng", "fang", "ya", "jian|qian", "kun", "an", "shen", \
    "duo|hui", "nao", "tu", "cheng", "yin", "hun|huan", "bi", "lian", "guo|wo", "die", \
    "zhuan", "hou", "bao|bu|pu", "bao", "yu", "di|shi|wei", "mao|wu|mou", "jie", "ruan|nuo", "ye|e|ai", \
    "geng", "chen|kan", "zong", "yu", "huang", "e", "yao", "yan", "bao|fu", "ji|ci", \
    "mei", "chang|dang|shang", "du|zhe", "tuo", "pou|yin", "feng", "zhong", "jie", "jin", "feng|heng", \
    "gang", "chun|chuan", "jian|kan", "ping", "lei", "jiang|xing", "huang", "leng", "duan", "wan", \
    "xuan", "ji|xi", "ji", "kuai", "ying", "da|ta", "cheng", "yong", "kai", "su", \
    "su", "shi", "mi", "da|ta", "weng", "cheng", "du|tu", "tang", "qiao|que", "zhong", \
    "li", "peng", "bang", "sai|se", "zang", "dui", "chen|tian|zhen", "wu", "zheng", "xun", \
    "ge", "zhen", "ai", "gong", "yan", "kan", "chen|tian|zhen", "yuan", "wen", "xie", \
    "liu", "hai", "lang", "shang|chang", "peng", "beng", "chen", "lu", "lu", "ou|qiu", \
    "jian|qian", "mei", "mo", "zhuan|tuan", "shuang", "shu", "lou", "chi", "man", "biao", \
    "jing", "ce", "shu|ye", "zhi|di", "zhang", "kan", "yong", "dian|nian", "chen", "zhi|zhuo", \
    "ji|xi", "guo", "qiang", "jin|qin", "di", "shang", "mu", "cui", "yan", "ta", \
    "zeng", "qian", "qiang", "liang", "wei", "zhui", "qiao", "ceng|zeng", "xu", "shan|chan", \
    "shan", "fa|ba|fei", "pu", "kuai|tui", "dong|tuan", "fan", "qiao|que", "mei|mo", "dun", "dun", \
    "zun|dun|cun", "di", "sheng", "duo|hui", "duo", "tan", "deng", "wu|mu", "fen", "huang", \
    "tan", "da", "ye", "zhu", "jian", "ao|yu", "qiang", "ji", "qiao|ao", "ken", \
    "yi|tu", "pi|bi", "bi", "dian", "jiang", "ye", "weng|yong", "xue|jue|bo", "dan|shan|tan", "lan", \
    "ju", "huai", "dang", "rang", "qian", "xun", "lan|xian", "xi", "he", "ai", \
    "ya", "dao", "hao", "ruan", "jin", "lei|lv", "kuang", "lu", "yan", "tan", \
    "wei", "huai|hui", "long", "long", "rui", "li", "lin", "rang", "chan", "xun", \
    "yan", "lei", "ba", "wan", "shi", "ren", "san", "zhuang", "zhuang", "qing|sheng", \
    "yi", "mai", "ke|qiao", "zhu", "zhuang", "hu", "hu", "kun", "yi|yin", "hu", \
    "xu", "kun", "shou", "mang", "dun", "shou", "yi", "zhe|zhong|dong|zhi", "ying|gu", "chu", \
    "jiang|xiang", "feng|pang", "bei", "zhai", "bian", "sui", "qun", "ling", "bi|fu", "cuo", \
    "jia|xia|yan", "xiong|xuan", "xie", "nao", "jia|xia", "kui", "xi|yi|yu", "wai", "yuan|wan", "mao|wan", \
    "su", "che|duo|zhi", "che|duo|zhi", "ye", "qing", "oes", "gou", "gou", "qi", "meng", \
    "meng", "yin", "huo", "chen", "da|dai", "ce|ze", "tian", "ta|tai", "fu", "jue|guai", \
    "wai|wo|yao", "yang|ying", "hang|ben", "gao", "shi|yi", "ben|tao", "tai", "tou", "tao|yan", "bi", \
    "yi", "kua", "jia|ga", "dui|duo", "hwa", "kuang", "yun", "jia|ga", "ba", "en", \
    "lian", "huan", "di|ti", "yan", "pao", "juan", "qi|ji", "nai", "feng", "xie|lie", \
    "fen|kang", "dian", "juan|quan", "kui", "cou|zou", "huan", "qi|xie", "kai", "she|zha", "ben|fen", \
    "yi", "jiang", "tao", "zang|zhuang", "ben", "xi", "huang", "fei", "diao", "xun", \
    "beng|keng", "dian|ding|ting|zun", "ao", "she", "weng", "ha|tai|po", "ao|you|yu", "wu", "ao", "jiang", \
    "lian", "duo|dui", "yun", "jiang", "shi", "fen", "huo", "bi", "luan", "duo|che", \
    "nv|ru", "nu", "ding|ting", "nai", "qian", "gan|jian", "chi|jie|ta", "jiu", "nuan", "cha", \
    "hao", "xian", "fan", "ji", "shuo|yue", "ru", "fei|pei", "wang", "hong", "zhuang", \
    "fu", "ma", "dan", "ren", "fu|you", "jing", "yan", "jie|ha|hai", "wen", "zhong", \
    "pa", "du", "ji", "hang|keng", "zhong", "jiao|yao", "jin|xian", "yun", "miao", "pei|pi|fou", \
    "chi", "jue|yue", "zhuang", "hao|niu", "yan", "na|nan", "xin", "fen", "bi", "yu", \
    "tuo", "feng", "yuan|wan", "fang", "wu", "kou|yu", "gui", "du", "ba|bo", "ni", \
    "chou|zhou", "zhuo", "zhao", "da", "ni|nai", "yuan", "tou", "xuan|xian|xu", "yi|zhi", "e", \
    "mei", "mo", "qi", "bi", "shen", "qie", "e", "he", "xu", "fa", \
    "zheng", "min", "ban", "mu", "fu", "ling", "zi", "zi", "shi", "ran", \
    "pan|shan|xian", "yang", "gan|man", "jie|ju|xu", "gu", "si", "sheng|xing", "wei", "ci|zi", "ju", \
    "pan|shan|xian", "pin", "ren", "tao|tiao|yao", "dong", "jiang", "shu", "ji", "gai", "xiang", \
    "hua|huo", "juan", "jiao|xiao", "gou|du", "lao|mu", "jian", "jian", "yi", "nian", "zhi", \
    "zhen", "ji|yi", "xian", "heng", "guang", "xun|jun", "kua|hu", "yan", "ming", "lie", \
    "pei", "e|ya", "you", "yan", "cha", "xian|shen", "yin", "ti|shi|ji", "gui|wa", "quan", \
    "zi", "song", "wei", "hong", "gui|wa", "lou|lv", "ya", "rao|yao", "jiao", "lian|luan", \
    "pin|ping", "dan|xian", "shao", "li", "cheng|sheng", "xie", "mang", "fu", "suo", "wu|mu", \
    "wei", "ke", "cu|chuo|lai", "cu|chuo", "ting|tian", "niang", "xing", "nan", "yu", "na|nuo", \
    "pou|bi", "sui|nei", "juan", "shen", "zhi", "han", "di", "zhuang", "e", "ping", \
    "tui", "xian|man", "mian|wan", "wu|yu", "yan", "wu", "ai|xi", "yan", "yu", "si", \
    "yu", "wa", "li", "xian", "ju", "ju|qu|shu", "zhui|shui", "qi", "xian", "zhuo", \
    "dong", "chang", "lu", "ai|e", "e", "e", "lou|lv", "mian", "cong", "pei|bu|pou", \
    "ju", "po", "cai", "ling", "wan", "biao", "xiao", "shu", "qi", "hui", \
    "fan|fu", "wo", "wo|rui", "tan", "fei", "fei", "jie|qie", "tian", "ni", "juan|quan", \
    "jing", "hun", "jing", "jin|qian", "dian", "xing", "hu", "guan|wan", "lai", "bi", \
    "yin", "chou|zhou", "chuo|nao", "fu", "jing", "lun", "an|nue", "lan", "kun|hun", "yin", \
    "ya", "ju", "li", "dian", "xian", "hua", "hua", "ying", "chan", "shen", \
    "ting", "yang|dang", "yao", "mou|mu|wu", "nan", "ruo|chuo", "jia", "tou|yu", "xu", "yu", \
    "wei", "di|ti", "rou", "mei", "dan", "ruan|nen", "qin|qing", "hui", "wo", "qian", \
    "chun", "miao", "fu", "jie", "duan", "pei|xi|yi", "zhong", "mei", "huang", "mian", \
    "an|yan", "ying", "xuan", "jie", "wei", "mei", "yuan", "zheng", "qiu", "ti|shi", \
    "xie", "duo|tuo", "lian", "mao", "ran", "si", "pian", "wei", "wa", "cu", \
    "hu", "yun|ao|wo", "jie|qie", "bao", "xu", "yu|tou", "gui", "zou|chu", "yao", "bi|pi", \
    "xi", "yuan", "sheng|ying", "rong", "ru", "chi", "liu", "mei", "pan", "yun|ao|wo", \
    "ma", "gou", "chou|kui", "qin|shen", "jia", "sao", "zhen", "yuan", "jie|suo", "rong", \
    "ming|meng", "ying|xing", "ji", "su", "niao", "xian", "tao", "bang|pang", "lang", "nao", \
    "bao", "ai", "pi", "pin", "yi", "biao|piao", "kou|yu", "lei", "xuan", "man|yuan", \
    "yi", "zhang", "kang", "yong", "ni", "li", "di", "gui|zui", "yan", "jin", \
    "zhuan|tuan", "chang", "ce|ze", "han|nan", "nen", "lao", "mo", "zhe", "hu", "hu", \
    "ao", "nen|ruan", "qiang", "ma", "pie", "gu", "wu", "jiao|qiao", "tuo|duo", "zhan", \
    "miao", "xian", "xian", "mo", "liao", "lian", "hua", "gui", "deng", "zhi", \
    "xu", "yi", "hua", "xi", "kui", "rao|yao", "xi", "yan", "chan", "jiao", \
    "mei", "fan|fu", "fan", "xian|yan|jin", "yi", "hei", "jiao", "fan|fu", "shi", "bi", \
    "chan|shan", "sui", "qiang", "lian", "huan|xuan|qiong", "xin", "niao", "dong", "yi", "can", \
    "ai", "niang", "ning", "ma", "tiao|diao", "chou", "jin", "ci", "yu", "pin", \
    "rong", "ru|nou", "er|nai", "yan", "tai", "ying", "can|qian", "niao", "yue", "ying", \
    "mian", "bi", "ma|mo", "shen", "xing", "ni", "du", "liu", "yuan", "lan", \
    "yan", "shuang", "ling", "jiao", "niang|rang", "lan", "xian|qian", "ying", "shuang", "xie|hui", \
    "quan|huan", "mi", "li", "luan|lian", "yan", "shu|zhu", "lan", "zi", "jie", "jue", \
    "jue", "kong", "yun", "ma|zi", "zi", "cun", "sun|xun", "fu", "bo|bei", "zi", \
    "xiao", "shen|xin", "meng", "si", "tai", "bao", "ji", "gu", "nu", "hua|jiao|xue", \
    "you", "zhuan|ni", "hai", "luan", "sun|xun", "nao", "me|mie", "cong", "qian|wan", "shu", \
    "chan|can", "ya", "zi", "yi|ni", "fu", "zi", "li", "hua|jiao|xue", "bo", "ru", \
    "nai", "nie", "nie", "ying", "luan", "mian", "ning|zhu", "rong", "ta|tuo|yi", "gui", \
    "che|du|zhai", "kong|qiong", "yu", "shou", "an", "tu|jia", "song", "kuan|wan", "rou", "yao", \
    "hong", "yi", "jing", "zhun", "mi|fu", "zhu", "dang", "hong", "zong", "guan", \
    "zhou", "ding", "wan|yuan", "yi", "bao", "shi|zhi", "shi", "chong|long", "pan|shen", "ke|qia", \
    "xuan", "shi", "you", "huan", "yi", "tiao", "shi|xi", "xian|xiong", "gong", "cheng", \
    "jiong|qun", "gong", "xiao", "zai", "zha", "bao|shi", "hai|he", "yan", "xiao", "jia|jie", \
    "pan|shen", "chen", "rong|yong", "huang", "mi", "kou", "kuan", "bin", "su|xiu", "cai", \
    "zan", "ji", "yuan", "ji", "yin", "mi", "kou", "qing|qiu", "he", "zhen", \
    "jian", "fu", "ning", "bing", "huan", "mei", "qin", "han", "yu", "shi", \
    "ning", "jin|qin", "ning", "zhi|tian", "yu", "bao", "kuan", "ning", "qin", "mo", \
    "cha|cui", "lou|lv|ju", "gua", "qin", "hu", "wu", "liao", "shi|zhi", "ning", "qian|se|zhai", \
    "pan|shen", "wei", "xie", "kuan", "hui", "liao", "jun", "huan|xian", "yi", "yi", \
    "bao", "qin", "chong|long", "bao", "feng", "cun", "dui", "shi|si", "xun|xin", "dao", \
    "lv|luo|lue", "dui", "shou", "po", "bian|feng", "zhuan", "fu|bu", "she|ye|yi", "kei|ke", "jiang|qiang", \
    "jiang|qiang", "shuan|tuan|zhuan", "wei|yu", "zun", "xin|xun", "shu|zhu", "dui", "dao", "xiao", "jie|ji", \
    "shao", "er|mi", "er|mi", "er|mi", "ga", "jian", "shu", "chen", "chang|shang", "chang|shang", \
    "ma|mo", "ga", "chang", "liao", "xian", "xian", "kun|hun", "pianpang|wang|you", "wang|you", "you", \
    "liao|niao", "liao|niao", "yao", "long|pang|mang|meng", "wang", "wang", "wang", "ga", "yao", "duo", \
    "kui", "zhong", "jiu", "gan", "gu", "gan", "tui|zhuai", "gan", "gan", "shi", \
    "yin|yun", "chi|che", "kao", "ni", "jin", "wei|yi", "niao|sui", "ju", "pi", "ceng", \
    "xie|xi", "bi", "ji|ju", "jie", "tian", "jue|que", "ti", "jie", "wu", "diao", \
    "shi", "shi|xi", "ping|bing", "ji", "xie", "zhen", "xie|xi", "ni", "zhan", "xi", \
    "uu", "man", "e|ke", "lou", "ping|bing", "ti|xie", "fei", "shu|zhu", "xie|ti", "tu", \
    "lv", "lv", "xi", "ceng", "lv", "ju", "xie", "ju", "jue", "liao", \
    "jue", "shu|zhu", "xie|xi", "cao|che", "tun|zhun", "ni|ji", "shan", "wa", "xian", "li", \
    "yan", "dao", "hui", "hong|long", "yi|ge", "qi", "ren", "wu", "an|han", "shen", \
    "yu", "chu", "sui|suo", "qi", "yen", "yue", "ban", "yao", "ang", "xia|ya", \
    "wu", "jie", "ji|e", "ji", "qian", "fen|cha", "wan", "qi", "cen", "qian", \
    "qi", "cha", "jie", "qu", "gang", "xian", "ao", "lan", "dao", "ba", \
    "zuo", "zuo", "yang", "ju", "gang", "ke", "gou", "xue", "po", "li", \
    "tiao", "zu|ju|qu", "yan", "fu", "xiu", "jia", "ling", "tuo", "pi", "ao", \
    "dai", "kuang", "yue", "qu", "hu", "po", "min", "an", "tiao", "ling", \
    "chi", "ping", "dong", "ceom", "kui|wei", "bang", "mao", "tong", "xue", "yi", \
    "bian", "he", "ke|ba", "luo", "e", "fu|nie", "xun", "die", "lu", "en", \
    "er", "gai", "quan", "tong|dong", "yi", "mu", "shi", "an", "wei", "huan", \
    "zhi|shi", "mi", "li", "ji", "tong|dong", "wei", "you", "gu", "xia", "lie", \
    "yao", "qiao|jiao", "zheng", "luan", "jiao", "e", "e", "yu", "xie|ye", "bu", \
    "qiao", "qun", "feng", "feng", "nao", "li", "you", "xian", "rong", "dao", \
    "shen", "cheng", "tu", "geng", "jun", "gao", "xia", "yin", "wu|yu", "lang|nang", \
    "kan", "lao", "lai", "xian|yan", "que", "kong", "chong", "chong", "ta", "lin", \
    "hua", "ju", "lai", "qi|yi", "min", "kun", "kun", "cui|zu", "gu", "cui", \
    "ya", "ya", "gang|bang", "lun", "lun", "ling", "jue|yu", "duo", "zheng", "guo", \
    "yin", "dong", "han", "zheng", "wei", "xiao", "bi|pi", "yan", "song", "jie", \
    "beng", "cui|zu", "jue|ku", "dong", "chan|zhan", "gu", "yin", "zi", "ze", "huang", \
    "yu", "wei|wai", "yang|dang", "feng", "qiu", "yang", "ti", "yi", "zhi|shi", "shi|die", \
    "zai", "yao", "e", "zhu", "kan|zhan", "lv", "yan", "mei", "han", "ji", \
    "ji", "huan", "ting", "cheng|sheng", "mei", "qian|kan", "mao|wu", "yu", "zong", "lan", \
    "jie|ke", "nie|yan", "yan", "wei", "zong", "cha", "sui|suo", "rong", "ke", "qin", \
    "yu", "qi|ti", "lou", "tu", "cui|dui", "xi", "weng", "cang", "tang|dang", "ying|rong", \
    "jie", "kai|ai", "liu", "wu", "song", "kao|qiao", "zi", "wei", "beng", "dian", \
    "ci|cuo", "qian|qin", "yong", "nie", "cuo|ci", "ji", "shi", "ruo", "song", "zong", \
    "jiang", "jiao|liao", "kang", "chan|yan", "die|di", "can|cen", "ding", "tu", "lou", "zhang", \
    "zhan|chan", "zhan|chan", "ao", "cao", "qu", "qiang", "wei|cui|zui", "zui", "dao", "dao", \
    "xi", "yu", "pei|pi", "long", "xiang", "ceng|zheng", "bo", "qin", "jiao", "yan", \
    "lao", "zhan", "lin", "liao", "liao", "jin|qin", "deng", "duo|tuo", "zun", "qiao|jiao", \
    "jue|gui", "yao", "jiao", "yao", "jue", "zhan|shan", "yi", "xue", "nao", "ye", \
    "ye", "yi", "nie", "xian|yan", "ji", "jie|xie", "ke", "gui|xi", "di", "ao", \
    "zui", "wei", "ni|yi", "rong", "dao", "ling", "jie", "yu", "yue", "yin", \
    "ru", "jie", "li|lie", "xi|gui", "long", "long", "dian", "ying|hong", "xi", "ju", \
    "chan", "ying", "kui|wei", "yan", "wei", "nao", "quan", "chao", "cuan", "luan", \
    "dian", "dian", "nie", "yan", "yan", "yan", "kui|nao", "yan", "chuan|shun", "kuai", \
    "chuan", "zhou", "huang", "jing|xing", "shun|xun|yan", "chao", "chao", "lie", "gong", "zuo", \
    "qiao", "ju|qu", "gong", "keo", "wu", "pu", "pu", "cha|chai|ci", "qiu", "qiu", \
    "ji|qi", "yi", "si|yi", "ba", "zhi", "zhao", "xiang|hang", "yi", "jin", "sun", \
    "quan", "phas", "xun", "jin", "fu|po", "za", "bi|yin", "fu|shi", "bu", "ding", \
    "shuai", "fan", "nie", "shi", "fen", "pa", "zhi", "xi", "hu", "dan", \
    "wei", "zhang", "nu|tang", "dai", "wa|mo", "pei|pi", "mo|pa", "tie", "bo|fu", "chen|lian", \
    "zhi", "zhou", "bo", "zhi", "di", "mo", "yi", "yi", "ping", "qia", \
    "juan", "ru", "shuai", "dai", "zhen|zheng", "shui", "qiao", "zhen", "shi", "qun", \
    "xi", "bang", "dai", "gui", "chou|dao", "ping", "zhang", "jian|san", "wan", "dai", \
    "wei", "chang", "qie|sha", "qi|ji", "ce|ze", "guo", "mao", "zhu|du", "hou", "zhen|zheng", \
    "zheng|xu", "mi", "wei", "wo", "bi|fu", "kai|yi", "bang", "ping", "die", "gong", \
    "pan", "huang", "tao", "mi", "jia", "teng", "hui", "zhong", "shen|shan|qiao", "man", \
    "man|mu", "biao", "guo", "ce|ze", "mu", "bang", "zhang", "jing", "chan", "fu", \
    "zhi", "wu|hu", "fan", "zhuang|chuang", "bi", "bi", "zhang", "mi", "qiao", "chan", \
    "fen", "meng", "bang", "chou|dao", "mie", "chu", "jie", "xian", "lan", "an|gan", \
    "beng|bing|pian|ping", "nian|ning", "jian", "bing", "bing", "nie|xing", "gan", "mi|yao", "huan", "yao|you", \
    "you", "ji", "guang|an", "bi|pi", "ting", "ze", "guang", "peng|zhuang", "me|mo", "qiang|qing", \
    "bi|pi", "qin|qi", "tun|dun", "chuang", "gui", "ya", "xin|ting|bai", "jie", "xu", "lu|lv", \
    "wu", "zhuang", "ku", "ying", "de|di", "pao", "dian", "ya", "miao", "geng", \
    "ci", "fu|zhou", "tong", "long|pang", "fei", "xiang", "yi", "zhi", "tiao", "zhi", \
    "xiu", "du|duo", "zuo", "xiao", "tu", "gui", "ku", "pang|mang|meng", "ting", "you", \
    "bu", "bing|ping", "cheng", "lai", "bei|bi", "cuo|ji", "an|e|yan", "shu|zhe|zhu", "kang", "yong", \
    "tuo", "song", "shu", "qing", "yu", "yu", "miao", "sou", "ce|ci", "xiang", \
    "fei", "jiu", "e", "gui|wei|hui", "liu", "xia|sha", "lian", "lang", "sou", "zhi", \
    "bu", "qing", "jiu", "jiu", "jin|qin", "ao", "kuo", "lou", "yin", "liao", \
    "xi|dai", "lu", "yi", "chu", "chan", "tu", "si", "xin|qian", "miao", "chang", \
    "wu", "fei|fa", "guang|an", "kos", "kuai", "bi", "qiang|se", "xie", "lan|lin", "lan|lin", \
    "liao", "lu|lv", "ji", "ying", "xian", "ting", "yong", "li", "ting", "pianpang|yin", \
    "shun|xun|yan", "yan", "ting", "di", "po", "jian", "hui", "nai", "hui", "gong|pianpang", \
    "nian", "kai", "bian|pan", "yi", "qi", "nong|long", "fen", "qu|ju", "nan|yan", "yi", \
    "zang", "bi", "yi", "yi", "er", "san", "shi|te", "er", "shi", "shi", \
    "gong", "di|diao", "yin", "hu", "fu", "hong", "wu", "di|ti", "chi", "jiang", \
    "ba", "shen", "di|ti|tui", "zhang", "jue|zhang", "tao", "fu", "di", "mi", "xian", \
    "hu", "chao", "nu", "jing", "zhen", "yi", "mi", "juan|quan", "wan", "shao", \
    "ruo", "yuan|xuan", "jing", "diao", "zhang", "jiang", "qiang|jiang", "peng", "dan|tan", "qiang|jiang", \
    "bi", "bi", "she", "dan|tan", "jian", "gou|kao", "ge", "fa", "bi", "kou", \
    "jian", "bie", "xiao", "tan|dan", "guo|kuo", "jiang|qiang", "hong", "mi|ni", "kuo|guo", "wan", \
    "jue", "xue|ji", "ji", "gui", "dang", "lu", "lu", "duan|shi|tuan", "hui", "zhi", \
    "hui", "hui", "yi", "yi", "yi", "yi", "huo|yue", "huo|yue", "xian", "xing", \
    "wen", "tong", "yan", "pan|yan", "yu", "chi", "cai", "biao", "diao", "ban|bin", \
    "bang|pang|peng", "yong", "miao|piao", "zhang", "ying", "chi", "chi|fu", "zhuo|bo", "yi|tuo", "ji", \
    "fang|pang", "zhong", "yi", "wang", "che", "bi", "di", "ling", "fu", "wang", \
    "zheng", "cu|zu", "wang", "jing", "dai", "xi", "xun", "hen", "yang", "huai|hui", \
    "lv", "hou", "jia|wa|wang", "cheng|zheng", "zhi", "xu", "jing", "tu", "cong", "cong|xi", \
    "lai", "cong", "de|dei", "pai", "si|xi", "uu", "ji", "chang", "zhi", "cong|zong", \
    "zhou", "lai", "wu|ya|yu", "xie", "jie", "jian", "shi|ti", "jia|xia", "bian|pian", "huang", \
    "fu", "xun", "wei", "pang|fang|bang", "yao", "wei", "xi", "zheng", "biao|piao", "chi|ti", \
    "de", "zheng|zhi", "zheng|zhi", "bie", "de", "chong|zhong", "che", "yao|jiao", "hui", "jiao", \
    "hui", "mei", "long", "rang|xiang", "bao", "qu|ju", "xin", "xin", "bi", "yi", \
    "le", "ren", "dao", "ding|ting", "gai", "ji", "ren", "ren", "chan|qian", "keng|tan", \
    "dao|te", "tui|tei|te", "gan|han", "qi|yi", "tai|shi", "cun", "zhi", "wang", "mang", "lie|xi", \
    "fan", "ying", "tian", "wen|min", "wen|min", "zhong", "chong", "wu", "ji", "wu", \
    "xi", "jie|jia", "you", "wan", "cong", "song|zhong", "kuai", "yu|shu", "bian", "qi|zhi", \
    "shi|qi", "cui", "chen|dan", "tai", "dun|tun|zhun", "qin|qian", "nian", "hun", "xiong", "niu", \
    "wang|kuang", "xian", "xin", "hang|kang", "hu", "kai|xi", "fen", "fu|huai", "tai", "song", \
    "wu", "ou", "chang", "chuang", "ju", "yi", "bao", "chao", "min|men", "pei", \
    "zuo|zha", "zen", "yang", "ju|kou", "ban", "nu", "nao|niu", "zheng", "bo|pa", "bu", \
    "zhan|tie", "gu|hu", "hu", "ju|qu", "da|dan", "lian|ling", "si|sai", "you|chou", "di", "dai|yi", \
    "yi", "die|tu", "you", "fu", "ji", "peng", "xing", "yuan|yun", "ni", "guai", \
    "bei|fei|fu", "xi", "bi", "you|yao", "qie", "xuan", "cong", "bing", "huang", "xu|xue", \
    "chu|xu", "pi|bi", "shu", "xi", "tan", "yong", "long|zong", "dui", "mo|mi", "ki", \
    "yi", "shi", "nen|nin", "shun|xun", "shi|zhi", "xi", "lao", "heng|geng", "kuang", "mou", \
    "zhi", "xie", "lian", "tiao|yao", "guang|huang", "die", "hao", "kong", "wei|gui", "heng", \
    "qi|xu", "jiao|xiao", "shu", "si", "kua|hu", "qiu", "yang", "hui", "hui", "chi", \
    "jia|qi", "yi", "xiong", "guai", "lin", "hui", "zi", "xu", "chi", "shang", \
    "nv", "hen", "en", "ke", "dong|tong", "tian", "gong", "quan|zhuan", "xi", "qia", \
    "yue", "peng", "ken", "de", "hui", "e|wu", "xiao", "tong", "yan", "kai", \
    "ce", "nao", "yun", "mang", "tong|yong", "yong", "juan|yuan", "bi|pi", "kun", "qiao", \
    "yue", "yu|shu", "yu|tu", "jie|ke", "xi", "zhe", "lin", "ti", "han", "hao|jiao", \
    "qie", "ti", "bu", "yi", "qian", "hui", "xi", "bei", "man|men", "yi", \
    "heng", "song", "quan|xun", "cheng", "kui|li", "wu", "wu", "you", "li", "liang|lang", \
    "huan", "cong", "nian|yi", "yue", "li", "nin", "nao", "e", "que", "xuan", \
    "qian", "wu", "min", "cong", "fei", "bei", "de", "cui", "chang", "men", \
    "li", "ji", "guan", "guan", "xing", "dao", "qi", "kong", "tian", "lun", \
    "xi", "kan", "gun", "ni", "qing", "chou|dao|qiu", "dun", "guo", "zhan", "jing|liang", \
    "wan", "yuan|wan", "jin", "ji", "lin|lan", "yu|xu", "huo", "he", "juan|quan", "tan|dan", \
    "ti", "ti", "nian|nie", "wang", "chuo|chui", "hu", "hun|men", "xi", "chang", "xin", \
    "wei", "hui", "e|wu", "rui|suo", "zong", "jian", "yong", "dian", "ju", "can", \
    "cheng", "de", "bei", "qie", "can", "dan|da", "guan", "duo|tuo", "nao", "yun", \
    "xiang", "chuan|gua|zhui", "die|tie", "huang", "chun", "qiong", "re|ruo", "xing", "ce", "bian", \
    "min|hun", "zong", "shi|ti", "qiao|qiu", "chou|jiu|qiao", "bei", "xuan", "wei", "ge", "qian", \
    "wei", "yu", "tou|yu", "bi", "xuan", "huan", "fen|min", "bi", "yi", "mian", \
    "yong", "he|qi|kai", "yang|tang|shang|dang", "yin", "e", "chen|xin|dan", "mao", "ke|qia", "ke", "yu", \
    "ai", "qie", "yan", "nuo|ruan", "gan|han", "wen|yun", "cong|song", "si|sai", "leng", "fen", \
    "ying", "kui", "kui", "que", "gong|hong", "yun", "su", "su|shuo", "qi", "yao", \
    "song", "huang", "ji", "gu", "ju", "chuang", "ni", "xie", "kai", "zheng", \
    "yong", "cao", "xun", "shen", "bo", "kai|xi", "yuan", "xi|xie", "hun", "yong", \
    "yang", "li", "sao|cao", "tao", "yin", "ci", "chu|xu", "qian|qie", "tai", "huang", \
    "wen|yun", "shen|zhen", "ming", "gong", "she", "cao|cong", "piao", "mu", "mu", "guo", \
    "chi", "can", "can", "can", "cui", "min", "ni|te", "zhang", "tong", "ao", \
    "shuang", "man", "guan", "que", "zao", "jiu", "hui", "kai", "lian", "ou", \
    "song", "jin|qin", "yin", "lv", "shang", "wei", "tuan", "man", "qian|xian", "she", \
    "yong", "qiang|qing", "kang", "chi|di", "zhi|zhe", "lou|lv", "juan", "qi", "qi", "yu", \
    "ping", "liao", "cong|song", "you", "chong", "zhi", "tong", "cheng", "qi", "qu", \
    "peng", "bei", "bie", "qiong", "jiao", "zeng", "chi", "lian", "ping", "kui", \
    "hui", "qiao", "cheng|zheng", "yin|xin", "yin", "xi", "xi", "dan|da", "tan", "duo", \
    "dui", "dun|tun|dui", "su", "jue", "ce", "jiao|xiao", "fan", "fen", "lao", "lao", \
    "chong|zhuang", "han", "qi", "xian", "min", "jing", "liao", "wu", "can", "jue", \
    "cu", "xian", "tan", "sheng", "pi", "yi", "chu", "xian", "nao|nang", "dan", \
    "tan", "jing", "song", "dan|han", "ji|jiao", "wei", "huan|xuan", "dong", "qin", "qin", \
    "ju", "cao|sao", "ken", "xie", "ying", "ao|yu", "mao", "yi", "lin", "se", \
    "jun", "huai", "men", "lan", "ai", "lan|lin", "yan|ye", "guo|kuo", "xia", "chi", \
    "yu", "yin", "dai", "meng", "yi|ni|ai", "meng", "dui", "qi|ji", "mo", "lan|xian", \
    "men", "chou", "zhi", "nuo", "nuo", "yan|chu", "yang", "bo", "zhi", "kuang", \
    "kuang", "you", "fu", "liu", "mie", "cheng", "hui|sui", "chan", "meng", "lai|lan", \
    "huai", "xuan", "rang", "chan", "ji", "ju", "huan|guan", "she", "yi", "lian", \
    "nan", "mi|mo", "tang", "jue", "gang|zhuang", "gang|zhuang", "gang|zhuang", "ge", "yue", "wu", \
    "jian", "xu|qu", "shu", "reng|rong", "xi|hu", "cheng", "wo", "jie", "ge", "can|jian", \
    "qiang|zang", "huo|yu", "qiang", "zhan", "dong", "cu|qi", "jia", "die", "cai", "jia", \
    "ji", "zhi", "kan|zhen", "ji", "kui", "gai", "deng", "zhan", "chuang|qiang", "ge", \
    "jian", "jie", "yu", "jian", "yan|you", "lu", "xi|hu", "zhan", "xi|hu", "xi|hu", \
    "chuo", "dai", "qu", "hu", "hu", "hu", "e", "shi|yi", "ti", "mao", \
    "hu", "li", "fang|pang", "suo", "bian|pian", "dian", "jiong", "jiong|shang", "yi", "yi", \
    "shan", "hu", "fei", "yan", "shou", "ti", "cai|zai", "zha|za", "qiu", "le|li", \
    "pi|pu", "ba|pa", "da", "reng", "fan|fu", "ru", "zai", "tuo", "zhang", "diao|di", \
    "kang|gang", "yu|wu", "wu|ku", "gan|han", "shen", "cha|chai", "tuo|chi|yi", "ge|gu", "kou", "wu", \
    "den", "qian", "zhi", "ren", "guang|kuo|tang", "men", "sao", "yang", "chou|niu|zhou", "ban|fen|huo", \
    "che", "rao|you", "xi|cha", "qin|qian", "ban|pan", "jia", "yu", "fu|pu", "ao|ba", "zhe|xi", \
    "pi", "qi|zhi", "kan|sun|zhi", "e", "den", "hua|zhao", "cheng|zhang|zheng", "ji|qi", "yan", "wang|kuang", \
    "bian", "chao|suo", "gou|ju", "wen", "gu|hu", "yue", "jue", "ba|pa", "qin", "shen|dan", \
    "zheng", "yun", "wan", "ne|ni", "yi", "shu", "zhua", "pou", "dou|tou", "dou", \
    "gang|kang", "zhe|she", "fu|pou", "fu|mo", "pao", "ba", "niu|ao", "ze", "tuan|zhuan", "kou", \
    "liu|lun", "qiang|cheng", "yun|jun", "hu", "bao", "bing", "zhi|zhai", "beng|peng", "nan", "ba|bu|pu", \
    "pi", "chi|tai", "tao|yao", "zhen", "zha", "yang", "bao|pao|pou", "he|qia", "ni", "she|ye", \
    "di|qi|zhi", "chi", "pei|pi", "jia", "mo|ma", "mei", "chen|shen", "jia|xia|ya", "chou", "qu", \
    "min", "zhu|chu", "jia|ya", "bi|fei|fu|pi", "zha|zhan", "zhu", "dan|jie", "chai|ca", "mu", "dian|nian", \
    "la", "bu|fu", "pao", "ban|pan", "bo|pai", "lin|ling", "na", "guai", "qian", "ju", \
    "tuo|ta", "ba|bie|bo|fa", "chi|tuo", "chi|tuo", "niu|ao", "ju|gou", "zhuo", "pin|pan|fan", "qiao|shao|zhao", "bai", \
    "bai", "di|zhi", "ni", "ju", "kuo", "long", "jian", "qia", "yong", "lan", \
    "ning", "bo|fa", "ze|zhai", "qian", "hen", "kuo|gua", "shi", "jie|jia", "zheng", "nin", \
    "gong|ju", "gong", "quan", "quan|shuan", "cun|zun", "zan|za", "kao", "yi|chi|hai", "xie", "ce|chuo|se", \
    "hui", "bing|pin", "zhuai|ye", "shi|she", "na", "bai", "chi", "gua", "die|zhi", "guang|kuo", \
    "duo", "duo", "zhi", "jia|qia|qie", "an", "nong", "zhen", "ge|he", "jiao", "kua|ku", \
    "dong", "ru|na", "diao|tao|tiao", "lie", "zha", "lu", "she|die", "wa", "jue", "lie", \
    "ju", "zhi", "lian|luan", "ya", "wo|zhua", "ta", "jia|xie", "nao|rao|xiao", "dang", "jiao|kao", \
    "zheng", "ji", "hui|hun", "xian", "yu", "ai", "tuo|shui", "nuo", "cuo|zuo", "bo", \
    "geng", "ti", "zhen", "cheng", "suo|sha", "suo|sa|sha", "keng|qian", "mei", "nong", "ju", \
    "bang|beng|peng", "jian", "yi", "ting", "shan|yan", "nuo|sui|ruo|rua", "wan", "jia|xie", "cha|zha", "feng|peng", \
    "ku|jiao", "wu", "jun", "qiu|jiu|ju", "tong", "hun|kun", "chi|huo", "tu|shu|cha", "zhuo", "fu|pou", \
    "lv|luo", "ba|bie", "gan|han|xian", "qiao|shao|xiao", "nie", "juan|yuan", "ze", "song|shu|sou", "ye|yu", "jue|zhuo", \
    "bu", "wan", "bu|pu|zhi", "zun", "zhuai|ye", "zhai", "lu", "sou", "shui|tuo", "lao", \
    "sun", "bang", "jian", "huan", "dao", "wei", "wan|yu", "qin", "feng|peng", "she", \
    "li|lie", "min", "men", "fu|bu", "ba|bai|bi", "ju", "dao", "luo|wo", "ai", "juan|quan", \
    "yue", "song|zong", "tian|chen", "chui|duo", "cha|jie|qie", "tu", "ben", "na", "nian|nie", "wei|wo|re|ruo", \
    "cu|zuo", "wo|xia", "qi", "hen|xian", "cheng", "dian", "sao", "lun", "qing|qian", "gang", \
    "duo|zhuo", "shou", "diao|nuo", "fu|pei|pou", "di", "zhang", "hun", "ji|yi", "tao", "qia", \
    "qi", "bai|pai", "shu", "qian|wan", "ling", "ye", "ya", "jue|ku", "zheng", "liang", \
    "gua", "ni|yi|nie|nai", "huo|xu", "yan|shan", "ding|zheng", "lue", "cai", "tan|xian", "che", "bing", \
    "cha|jie|sha|xie", "ti", "kong|qiang", "tui", "yan", "ci|cuo|ze", "chou|zhou|zou", "ju", "tian", "qian", \
    "ken", "bai|bo", "pa|shou", "jie", "lu", "guo|guai", "ming", "geng|jie", "zhi", "dan|shan", \
    "meng", "chan|can|shan", "sao", "guan", "peng", "chuan|yuan", "nuo", "jian", "zheng|keng", "you|jiu", \
    "jian|qian", "chou|shu|you|yu", "yan", "kui", "nan", "hong|xuan", "rou", "pi|che", "wei", "sai|cai", \
    "cou|zou", "xuan", "mao|miao", "ti|di", "nie", "cha|zha", "shi", "song|zong", "zhen", "ji|yi", \
    "xun", "huang|yong", "bian", "yang", "huan", "yan", "zan|zuan", "an|yan|ye", "xu|ju", "ya", \
    "ou|wo", "ke|qia", "chuai|zhui", "ji", "di|ti", "la", "la", "cheng|chen", "jia|kai", "jiu", \
    "jiu", "tu", "jie|qi", "hui|hun", "gen", "chong|dong", "xian|xiao", "die|ye|she", "jia|xie", "huan|yuan", \
    "jian|qian", "ye", "cha|zha", "zha", "bei", "yao", "wei", "dem", "lan", "wen|wu", \
    "qin", "chan|shan", "ge", "lou", "zong", "gen", "jiao", "gou", "qin", "rong", \
    "huo|que", "zou|chou", "chi|chuai|yi", "zhan", "sun", "sun", "bo", "chu", "rong|nang", "bang|peng", \
    "chai|cuo|guo", "sao", "ke|e", "yao", "dao", "zhi", "nuo|nu|nou", "xie|la", "jian|lian", "shao|sou|xiao", \
    "qiu", "gao|kao|qiao", "xian", "shuo", "sang", "jin", "mie", "yi|e", "dui|chui", "nuo", \
    "shan", "da|ta", "jie|zhe|zha", "tang", "ban|pan|po", "ban|su", "da|ta", "li", "tao", "hu|ku", \
    "zhi|nai", "wa", "xia|hua", "qian", "wen", "qiang|cheng", "shen|tian", "zhen", "e", "xie", \
    "na|nuo", "quan", "cha", "zha", "ge", "wu", "en", "nie|sha|she|zhe", "gang|kang", "she|nie", \
    "lu|shu", "bai", "yao", "bin", "rong", "nan|tan", "sha|sa|shai", "chan|sun", "suo", "liao|nao|jiao|jiu|liu", \
    "chong", "chuang", "guo|guai", "bing", "feng|peng", "shuai", "tu|zhi|di", "ji|qi|cha", "song|sou", "zhai", \
    "lian", "cheng", "chi", "guan", "lu", "luo", "lou", "zong", "gai|xi", "hu|chu", \
    "zhua|zha", "cheng|qiang", "tang", "hua", "cui|cuo|zui", "nai|zhi", "mo|ma", "jiang|qiang", "gui", "ying", \
    "zhi", "ao|qiao", "zhi", "che|nie", "man", "can|chan", "kou", "chu|chi", "she|se|su|mi", "tuan|zhuan", \
    "chao|jiao", "mo", "mo", "zhe|la", "chan|can|shan|xian", "qian|keng", "biao|pao|piao", "jiang", "yao", "gou", \
    "qian", "liao", "ji", "ying", "jue", "bie|pie", "bie|pie", "lao", "dun", "xian", \
    "ruan|ruo", "gui", "zan|qian|zen", "yi", "xun|xian", "cheng", "cheng", "sa", "nao|xiao", "hong", \
    "si|xi", "han|qian", "heng|guang", "da|ta", "zun", "nian", "lin", "cheng|zheng", "wei|hui", "zhuang", \
    "jiao|kao", "ji", "cao", "tan|dan", "dan|shan", "che", "bo|fa", "che", "jue", "xiao|sou", \
    "lao|liao", "ben", "fu|mo", "qiao", "bo", "cuo|zuo", "zhuo", "suan|xuan|zhuan", "zhui|tuo|wei", "pu|bu", \
    "qin", "dun", "nian", "hua", "xie", "lu", "jiao", "cuan", "ta", "han", \
    "ji|yao|qiao", "wo|zhua", "jian|lian", "gan", "yong", "lei", "nang", "lu", "shan", "zhuo", \
    "ze|zhai", "bu|pu", "chuo", "ji|xi", "dang", "se", "cao", "qing", "jing|qing", "huan|juan|xuan", \
    "jie", "qin", "kuai", "dan|shan", "xie", "jia|qia|ye", "pi|bo", "bo|bai", "ao", "ju", \
    "ye", "e", "meng", "sou", "mi", "ji", "tai", "zhuo", "chou|dao", "xing", \
    "lan", "ca", "ju", "ye", "nou|nu|ru", "ye", "ye", "ni", "hu|huo", "jie", \
    "bin", "ning", "ge", "zhi", "jie|zhi", "guang|kuo|tang", "mi|mo", "jian", "xie", "la|lie", \
    "tan", "bai", "sou", "lu", "lue|li", "rao", "zhi|ti", "pan", "yang", "lei", \
    "ca|sa", "lu|shu", "cuan", "nian", "xian", "jun|pei", "huo|que", "li", "lai|la", "huan", \
    "ying", "lu|luo", "long", "qian", "qian", "zan|cuan", "qian", "lan", "xian|jian", "ying", \
    "mei", "ning|rang|xiang", "chan|shan", "ying", "cuan", "xie", "nie|sha|she|zhe", "luo", "mei", "mi|mo", \
    "chi", "zan|cuan", "luan|lian", "tan|nan", "zuan", "li|shai", "dian", "wa", "dang|tang", "jiao", \
    "jue", "lan", "li|luo", "nang", "qi|zhi", "gui", "gui", "ji|qi|yi", "xun", "po|pu", \
    "fan|pu", "shou", "kao", "you", "gai", "yi", "gong", "gan|han", "ban", "fang", \
    "zheng", "po", "dian", "kou", "fen|min", "wu|mou", "gu", "he|shi", "ce", "xiao", \
    "mi", "shou|chu", "ge|guo|e", "di|hua", "xu", "jiao", "min", "chen", "jiu", "zhen", \
    "dui|duo", "yu", "chi|sou", "ao", "bai", "xu", "jiao", "dui|duo", "lian", "nie", \
    "bi", "chang|cheng|zheng", "dian", "duo|que", "yi", "gan", "san", "ke", "yan|jiao", "dun|dui", \
    "qi|ji|yi", "tou", "xue|xiao", "duo|que", "jiao|qiao", "jing", "yang", "xia|gui", "min", "shu|shuo", \
    "ai|zhu", "qiao", "ai|zhu", "zheng", "di", "chen|zhen", "fu", "shu|shuo", "liao", "ou|qu", \
    "xiong|xuan", "yi", "jiao", "shan", "jiao", "zhu|zhuo", "yi|du", "lian", "bi", "li|tai", \
    "xiao", "xiao", "wen", "xue", "qi", "qi", "zhai", "bin", "jue|jiao", "zhai", \
    "uu", "fei", "ban", "ban", "lan", "yu|zhong", "lan", "men|wei", "dou|zhu", "sheng", \
    "liao", "jia", "hu", "cha|xie|ye", "jia", "yu", "zhen", "jiao", "wo|guan", "tou|tiao", \
    "dou", "jin", "che|chi|zhe", "yin|zhi", "fu", "qiang", "zhan", "qu", "chuo|zhuo", "zhan", \
    "duan", "zhuo", "shi|si", "xin", "zhuo", "zhuo", "jin|qin", "lin", "zhuo", "chu", \
    "duan", "zhu", "fang|pang", "jie|chan", "hang", "yu|wu", "shi|yi", "pei", "you|liu", "myeo", \
    "bang|beng|pang|peng", "qi", "zhan", "mao|wu", "lv", "pei", "pi|bi", "liu", "fu", "fang", \
    "xuan", "jing", "jing", "ni", "cou|sou|zou|zu", "zhao", "yi", "liu", "shao", "jian", \
    "eos", "yi", "qi", "zhi", "fan", "piao", "fan", "zhan", "kuai", "sui", \
    "yu", "wu|mo", "ji", "ji|xi", "ji", "huo", "ri", "dan", "jiu", "zhi", \
    "zao", "xie", "tiao", "jun|xun", "xu", "ga|xu", "la", "gan|han", "han", "tai|ying", \
    "di", "xu", "chan", "shi", "kuang", "yang", "shi", "wang", "min", "min", \
    "tun|zhen", "chun", "wu", "yun", "bei", "ang|yang", "ze", "ban", "jie", "hun|kun", \
    "sheng", "hu", "fang", "hao", "jiong|gui", "chang", "xuan", "meng|ming", "hun", "fen", \
    "qin", "hu", "yi", "cuo|xi", "cuan|xin", "yan", "ze", "fang", "tan|yu", "shen", \
    "ju", "yang", "zan", "bing|fang", "xing", "yang|ying", "xuan", "po|pei", "zhen", "ling", \
    "chun", "hao", "mei|mo|wen", "zuo", "mo", "bian", "xiong|xu", "hun", "zhao", "zong", \
    "shi|ti", "shi|xia", "yu", "fei", "yi|die", "mao", "ni|zhi", "chang", "on|wen", "dong", \
    "ai", "bing", "ang", "zhou", "long", "xian", "kuang", "tiao", "chao|zhao", "shi", \
    "huang", "huang", "xuan", "kui", "kua|xu", "jiao", "jin", "zhi", "jin", "shang", \
    "tong", "hong", "yan", "gai", "xiang", "shai", "xiao", "ye", "yun", "hui", \
    "han", "han", "jun", "wan", "xian", "kun", "zhou", "xi", "cheng|sheng", "sheng", \
    "bu", "zhe", "zhe", "wu", "wan|han", "hui", "hao", "chen", "wan", "tian", \
    "zhuo", "zui", "zhou", "pu", "jing|ying", "xi", "shan", "ni", "xi", "qing", \
    "du|qi", "jing", "gui", "zheng", "yi", "zhi", "yan|an", "wan", "lin", "liang", \
    "chang|cheng", "wang", "xiao", "zan", "fei", "xuan", "geng|xuan", "yi", "jia|xia", "yun", \
    "hui", "xu", "min", "kui", "ye", "ying", "du|shu", "wei", "shu", "qing", \
    "mao", "nan", "jian|lan", "nuan|xuan", "an", "yang", "chun", "yao", "suo", "pu", \
    "ming", "jiao", "kai", "gao|hao", "weng", "chang", "qi", "hao", "yan", "li", \
    "ai", "ji|jie", "ji", "men", "zan", "xie", "hao", "mu", "mo|mu", "cong", \
    "ni", "zhang", "hui", "bao|bo|pu", "han", "xuan", "chuan", "liao", "xian", "tan", \
    "jing", "pie", "lin", "tun", "xi", "yi", "ji|jie", "huang", "dai", "ye", \
    "ye", "li", "tan", "tong", "xiao", "fei", "shen", "zhao", "hao", "yi", \
    "shan|xiang", "xing", "shan", "jiao", "bao", "jing", "yan", "ai", "ye", "ru", \
    "shu", "meng", "xun", "yao", "bao|pu", "li", "chen", "kuang", "die", "uu", \
    "yan|yao", "huo", "lu|lv", "xi", "rong", "long", "nang", "luo", "luan", "shai", \
    "tang", "yan", "zhu", "yue", "yue|zad", "qu", "ye", "geng", "yi", "hu", \
    "e|he", "shu", "cao", "cao", "sheng", "man", "zeng|ceng", "zeng|ceng", "ti", "zui|cuo", \
    "jian|can|qian", "xu", "hui|kuai", "yin", "qie", "fen", "pi", "ru|yue", "wei|you", "ruan|wan", \
    "peng", "fen|ban", "bi|bo|fu", "ling", "ku|fei", "xu|qu|chun", "uu", "nv", "tiao", "shuo", \
    "zhen", "lang", "lang", "juan|zui", "ming", "mang|huang", "wang", "tun", "chao|zhao", "qi", \
    "qi|ji", "ying", "zong", "wang", "tong|chuang", "lang", "lao", "mang|meng", "long", "mu", \
    "pin|teun", "wei", "mo", "ben|pen", "ya|zha", "shu|zhu", "shu|zhu", "teul", "shu|zhu", "ren", \
    "ba", "piao|pu|po", "duo", "duo", "dao|mu", "li", "qiu|gui", "ji|wei", "jiu", "bi", \
    "xiu", "zhen|cheng", "ci", "sha", "ru", "duo|za", "quan", "qian", "yu|wu", "gan", \
    "wu", "cha", "shan|sha", "xun", "fan", "wo|wu", "zi", "li", "xing", "cai", \
    "cun", "er|ren", "shao|biao", "tuo|zhe", "duo|di", "zhang", "mang", "chi", "yi", "ge|gu|gai", \
    "gong", "du|tu", "li|yi", "qi", "shu", "gang|gong", "tiao", "jiang", "shan", "wan", \
    "lai", "jiu", "mang", "yang", "ma", "miao", "zhi|si|xi", "yuan", "hang|kang", "fei|bei", \
    "bei", "jie", "dong", "gao", "yao", "qian|xian", "chu", "chun", "ba|pa", "shu|dui", \
    "hua", "xin", "niu|chou", "shu|zhu", "chou", "song", "ban", "song", "ji", "yue|wo", \
    "jin", "gou", "ji", "mao", "bi|pi", "bi|pi|mi", "kuang|wang", "ang", "fang|bing", "fen", \
    "yi", "fu", "nan", "si|xi", "hu|di", "ya|ye", "dou|zhu", "xin", "chen|zhen", "yao", \
    "lin", "nen|rui", "e", "mei", "zhao", "guan|guo|luo", "zhi|qi", "cong|zong", "yun", "zui", \
    "sheng", "shu", "zao", "duo|di", "li", "lu", "jian", "cheng", "song", "qiang", \
    "feng", "zhan|nan", "xiao", "zhen|xian", "gu|ku", "ping", "tai|si|ci", "xi", "zhi", "guai", \
    "xiao", "jia", "jia", "gou|ju", "bao|fu", "mo", "xie|yi", "ye", "ye", "shi", \
    "nie", "bi", "tuo|duo", "yi|li", "ling", "bing", "ni|chi", "la", "he", "ban|pan", \
    "fan", "zhong", "dai", "ci", "yang|ying", "fu", "bai|bo", "mei|mou", "gan|qian", "qi", \
    "ran", "rou", "mao|shu", "shao", "song", "zhe", "jian|xia", "you|zhou", "shen", "gui|ju", \
    "tuo", "zha|zuo", "ran|nan", "chu|ning|zhu", "yong", "chi|di", "zhi|die", "zu|ju|zha", "zha|cha", "dan", \
    "gu", "bu|pu", "jiu", "ao", "fu", "jian", "ba|bo|bie|fu|pei", "duo|zuo|wu", "ke", "nai", \
    "zhu", "bi|bie", "liu", "chai|ci|zhai", "zha|shan", "si", "zhu", "pei|bei", "shi|fei", "guai", \
    "zha|cha", "yao", "cheng|jue", "jiu", "shi", "zhi", "liu", "mei", "li", "rong", \
    "zha|shan|ce|shi", "zao", "biao", "zhan", "zhi", "long", "dong", "lu", "saeng", "li|yue", \
    "lan", "yong", "shu|sun", "xun|sun", "shuan", "qi|qie", "chen|zhen", "qi|xi", "li|lie", "yi", \
    "xiang", "zhen", "li", "ci|se", "kuo|gua|tian", "kan", "bing|ben", "ren", "xiao|jiao", "bai", \
    "ren", "bing", "zi", "chou", "xie|yi", "ci", "xu|yu", "zhu", "zun|jian", "zui", \
    "er", "er", "you|yu", "fa", "gong", "kao", "lao", "zhan", "lie|li", "yin", \
    "yang", "he|hu", "gen", "zhi|yi", "shi", "ge|he|luo", "zai|zhi", "luan", "fu", "jie", \
    "heng|hang", "gui", "tao|tiao|zhao", "guan|guang", "gui|wei", "kuang", "ru", "an", "an", "juan|quan", \
    "yi|ti", "zhuo", "ku", "zhi", "qiong", "dong|tong", "sang", "sang", "huan", "ju|jie", \
    "jiu", "xue", "duo", "chui", "mou|yu", "zan|za", "uu", "ying", "jie", "liu", \
    "zhan", "ya", "rao|nao", "zhen", "dang", "qi", "qiao", "hua", "hui|gui", "jiang", \
    "zhuang", "xun", "suo", "sa", "chen|zhen", "bei", "ying|ting", "kuo", "jing", "bo|po", \
    "ben|fan", "fu", "rui", "tong", "jue", "xi", "lang", "liu", "feng", "qi", \
    "wen", "jun", "gan|han", "su|yin", "liang", "qiu", "ting", "you", "mei", "bang", \
    "long", "peng", "zhuang", "di", "juan|xuan|xue", "tu|cha", "zao", "ao|you", "gu", "bi", \
    "di", "han", "zi", "zhi", "ren", "bei", "geng", "jian|xian", "huan", "wan", \
    "nuo", "jia", "tiao", "ji", "xiao", "lv", "hun|kuan", "shao|sao", "cen", "fen", \
    "song", "meng", "wu|yu", "li", "li|si|qi", "dou", "qin", "ying", "suo|xun", "ju", \
    "ti", "xie", "kun|hun", "zhuo", "shu", "chan", "fan", "wei", "jing", "li", \
    "bing|bin", "xia", "fo", "tao|chou|dao", "zhi", "lai", "lian", "jian", "tuo|zhuo", "ling", \
    "li", "qi", "bing", "lun", "song|cong", "qian", "mian", "qi", "ji|qi", "cai", \
    "ao|gun|hun", "chan", "de|zhe", "fei", "pai|pei|bei", "bang", "bei|pou|bang", "hun", "zong", "chang|cheng", \
    "zao", "ji", "li|lie", "peng", "yu", "yu", "gu", "gun|jun", "dong", "tang", \
    "gang", "wang", "di|ti|dai", "cuo|que", "fan", "cheng", "chen|zhan", "qi", "yuan", "yan", \
    "yu", "juan|quan", "yi", "sen", "shen|ren", "chui|duo", "leng|ling", "qi|xi", "zhuo", "fu|su", \
    "ke|kuan", "lai", "zou|sou", "zou", "zhuo|zhao", "guan", "fen", "fen", "chen", "qing", \
    "ni|nie", "wan", "guo", "lu", "hao", "jie|qie", "yi", "chou|zhou|diao", "ju", "ju", \
    "cheng|sheng", "zu|cui", "liang", "kong|qiang", "zhi", "zhui|chui", "ya|e", "ju", "bei|pi", "jiao", \
    "zhuo", "zi", "bin", "peng", "ding", "chu", "chang", "men", "hua", "jian", \
    "gui", "xi", "du", "qian", "dao", "gui", "dian", "luo", "zhi", "juan|quan", \
    "myeong", "fu", "geng", "peng", "shan", "yi", "tuo", "san|sen", "chuan|duo", "ye", \
    "fu", "wei|hui", "wei", "duan", "jia", "zong", "jian|han", "yi", "shen|zhen", "po|xi", \
    "yan|ya", "yan", "chuan", "jian|zhan", "chun", "yu", "he", "zha|cha", "wo", "pian", \
    "bi", "yao", "guo|huo|kua", "xu", "ruo", "yang", "la", "yan", "ben", "hui", \
    "kui", "jie", "kui", "si", "feng|fan", "xie", "tuo", "ji|zhi", "jian", "mu", \
    "mao", "chu", "ku|hu", "hu", "lian", "leng", "ting", "nan", "yu", "you", \
    "mei", "cong|song", "yuan|xuan", "xuan", "yang", "zhen", "pian", "ye|die", "ji", "jie|qia", \
    "ye", "chu|zhu", "dun|shun", "yu", "zou|cou", "wei", "mei", "di|ti", "ji", "jie", \
    "kai|jie", "qiu", "ying", "rou", "huang", "lou", "le|yue", "quan", "xiang", "pin", \
    "shi", "gai|gui|jie", "tan", "lan", "wen|yun", "yu", "chen", "lv", "ju", "shen", \
    "chu", "pi", "xie", "jia", "yi", "zhan|chan|zhen", "bo|fu", "nuo", "mi", "lang", \
    "rong", "gu", "jin|jian", "ju", "ta", "yao", "zhen", "bang|beng", "sha|xie", "yuan", \
    "zi", "ming", "su", "jia", "yao", "jie", "huang", "gan|han", "fei", "zha", \
    "qian", "ma", "sun", "yuan", "xie", "rong", "shi", "zhi", "cui", "wen", \
    "ting", "liu", "rong", "tang", "que", "zhai", "si", "sheng", "ta", "ke", \
    "xi", "gu", "qi", "gao|kao", "gao|kao", "sun", "pan", "tao", "ge", "chun", \
    "zhen|dian", "nou", "ji", "shuo", "gou|jie", "chui|zhui", "qiang|cheng", "cha", "qian|xian|lian", "huai", \
    "mei", "chu|xu", "gang", "gao", "zhuo", "tuo", "qiao", "yang", "zhen|dian", "jia", \
    "kan|jian", "zhi", "dao", "long", "bin|bing", "zhu", "sang", "xi|die", "ji|gui", "lian", \
    "hui", "yong|rong", "qian", "guo", "gai", "gai", "tuan|quan|shuan", "hua", "qi|se", "shen|sen", \
    "cui|zui|zhi", "peng", "you|chao", "hu", "jiang", "hu", "huan", "gui", "nie|xie|yi", "yi", \
    "gao", "kang", "gui", "gui", "cao|zao", "man|wan", "jin|qin", "di|zhe|zhi", "chong|zhuang", "le|yue", \
    "lang", "chen", "cong|zong", "li|chi", "xiu", "qing", "shuang|shang", "fan", "tong", "guan", \
    "ze", "su", "lei", "lu", "liang", "mi", "lou|lv", "chao|jiao", "su", "ke", \
    "chu", "cheng|tang", "biao", "lu|du", "liao|jiu", "zhe", "zha", "du|shu", "zhang", "lang|man", \
    "mo|mu", "niao|mu", "yang", "tiao", "peng", "zhu", "sha|xie", "xi", "quan", "guang|heng", \
    "jian", "cong", "ji", "yan", "qiang", "xue", "ying", "er|zhi", "xun", "zhi|yi", \
    "qiao", "zui", "cong", "pu", "shu", "hua", "gui|kui", "zhen", "zun", "yue", \
    "shan", "xi", "chun", "dian", "fa|fei", "gan", "mo", "wu", "qiao", "rao|nao", \
    "lin", "liu", "jiao|qiao", "jian|xian", "run", "fan", "zhan|jian", "du|luo|tuo", "liao|lao", "yun", \
    "shun", "dun|tui", "cheng", "tang|cheng", "meng", "ju", "chen|cheng|deng", "su|xiao|qiu", "jue", "jue", \
    "tan|dian", "hui", "ji", "nuo", "xiang", "duo|tuo", "ning", "rui", "zhu", "tong|chuang", \
    "zeng|ceng", "fei|fen", "qiong", "ran|yan", "heng", "qian|qin", "gu", "liu", "lao", "gao", \
    "chu", "xi", "sheng", "ca|zi", "san|zan", "ji", "dou", "jing", "lu", "jian", \
    "chu", "yuan", "da", "qiao|shu", "jiang", "shan|tan", "lin", "nao", "yin", "xi", \
    "hui", "shan", "zui", "xuan", "cheng", "gan", "ju", "zui", "yi", "qin", \
    "pu", "dan|yan", "lei", "feng", "hui", "dang", "ji", "sui", "bo|nie", "bo|ping", \
    "cheng", "chu", "zhua", "hui|gui", "ji", "jie", "jia", "jing|qing", "zhai|shi", "jian", \
    "qiang", "dao", "yi", "biao", "song", "she", "lin", "li", "cha|sa", "meng", \
    "yin", "tao|chou|dao", "tai", "mian", "qi", "tuan", "bin|bing", "huo|hua", "ji", "lian|qian", \
    "mi|ni", "ning", "yi", "gao", "jian|kan", "yin", "ru|ruan|nou", "qing", "yan", "qi", \
    "mi", "di|zhao", "gui", "chun", "ji", "kui", "po", "deng", "chu", "ge", \
    "mian", "you", "zhi", "guang|huang|gu|guo", "qian", "lei", "lei|lie", "sa", "lu", "li", \
    "cuan", "lv|chu", "mie|mei", "hui", "ou", "lv", "zhi", "gao", "du", "yuan", \
    "yue|li", "fei", "zhu|zhuo", "sou", "lian", "jiang", "chu", "qing", "zhu", "lu|lv", \
    "yan", "li", "zhu", "qin|chen", "ji|jie|jue", "e", "su", "huai|gui", "nie", "yu", \
    "long", "lai|la", "jiao|qiao", "xian", "gui|kwi", "ju", "xiao|qiu", "ling", "ying", "jian|shan", \
    "yin", "you", "ying", "rang|xiang", "nong", "bo", "chan|zhan", "lan|lian", "ju", "shuang", \
    "she", "zui|wei", "cong", "guan|quan", "qu", "cang", "jou", "yu", "luo", "li", \
    "cuan|zuan", "luan", "dang|tang", "jue|qu", "eom|yan", "lan", "lan", "zhu", "lei|luo", "li", \
    "ba", "nang", "yu", "ling", "guang", "qian", "ci|zi", "huan", "xin", "yu", \
    "huan|yu|yi", "qian|xian", "ou", "xu", "chao", "chu|qu|xi", "qi", "ke|ai|kai", "yin|yi", "jue", \
    "kai|xi", "xu", "he|xia", "yu", "kui", "lang", "kuan|xin", "sou|shuo", "xi", "ai|ei", \
    "qi|yi", "qi", "xu|chua", "chuai|chi", "qin|yin", "kuan|xin", "dan|kan|qian", "kuan|xin", "kan|ke", "chuan", \
    "sha|xia", "gua", "yin", "xin", "xie|ya", "yu", "qian", "xiao", "ye", "ge", \
    "wu|yang", "tan", "jin|qun", "ou", "hu", "ti|xiao", "huan", "xu", "fen|pen", "xi|yi", \
    "chi|xiao", "xu|chua", "xi|she", "uu", "lian|han", "chu", "yi", "e", "yu", "chuo", \
    "huan", "zheng|zhi", "zheng", "ci", "bu", "wu", "qi", "bu", "bu", "wai", \
    "ju", "qian", "zhi|chi", "se", "chi", "se", "zhong", "sui|suo", "sui", "li", \
    "ze|ji", "yu", "li", "gui|kui", "dai|e", "e", "si", "jian", "zhe", "wen|mo", \
    "mo", "yao", "mo|wen", "cu", "yang", "tian", "sheng", "dai", "shang", "xu", \
    "xun", "shu", "can", "jing|jue", "bi|piao", "qia", "qiu", "su", "jing|qing", "yun", \
    "lian", "yi", "bo|fou|tou|ye", "zhi|shi", "yan|ye", "can", "hun|men|mei", "dan", "ji", "die", \
    "zhen", "yun", "wen", "chou", "bin", "ti", "jin", "shang", "yin", "diao", \
    "jiu", "kui|hui", "cuan", "yi", "dan", "du", "jiang", "lian", "bin", "du", \
    "jian", "jian", "shu", "ou", "duan", "zhu", "yin|yan", "sheng|qing|keng", "yi", "sa|shai|xie", \
    "ke|qiao", "ke|qiao", "yao|xiao", "xun", "dian", "hui", "hui", "gu", "qiao|que", "ji|qi", \
    "yi", "kou|qu", "hui", "duan", "yi", "xiao", "mou|wu", "wan|guan", "mu|wu", "mei", \
    "mei", "ai", "jie", "dai|du", "yu", "bi|pi", "bi", "bi", "pi", "pi", \
    "bi", "chan", "mao", "uu", "uu", "bi|pi", "mao|lie", "jia", "zhan", "sai", \
    "mu|mao", "tuo", "xun", "er", "rong", "xian", "ju", "mu", "hao", "qiu", \
    "dou|nuo", "uu", "tan", "pei", "ju", "duo", "cui|qiao|xia", "bi", "san", "san", \
    "mao", "sai|sui", "shu", "shu", "tuo", "he|ke", "jian", "ta", "san", "lv|shu", \
    "mu", "li|mao", "tong", "rong", "chang", "pu", "lu", "zhan", "sao", "zhan", \
    "meng", "lu", "qu", "die", "shi|zhi", "di|zhi", "min", "jue", "mang|meng", "qi", \
    "pie", "nai", "qi", "dao", "xian", "chuan", "fen", "ri|yang", "nei", "bin", \
    "fu", "shen", "dong", "qing", "qi|xi", "yan|yin", "xi", "hai", "yang", "an", \
    "ya", "ke", "qing", "ya", "dong", "dan", "lv", "qing", "yang", "yun", \
    "yun", "shui", "shui", "cheng|zheng", "bing", "yong", "dang", "shui", "le", "ni|mei", \
    "qiu|tun", "fan", "jiu|qiu|gui", "ding|ting", "shi|xie|zhi", "qiu", "pa|bin", "ze", "mian", "cuan", \
    "hui", "diao", "han", "cha", "zhuo|yue|que", "chuan", "wan|huan", "fan|fa", "da|tai|dai", "xi", \
    "tuo", "mang", "you|qiu", "qi", "shan|shuan", "chi|pin", "gan|han", "qian", "yu|wu", "wu|yu", \
    "xun", "si", "ru", "gong", "jiang", "che|chi|tuo", "wa|wu|yu", "tu", "jiu", "tang|shang", \
    "zhi|ji", "zhi", "qian", "mi", "gu|yu", "hong|wang", "jing", "jing", "rui|tun", "jun", \
    "hong", "tai", "fu|quan", "ji", "bian", "bian", "han|gan", "wen|men", "zhong", "fang|pang", \
    "xiong", "jue|que|xue", "hu|huang", "niu|you", "gai|qi|yi", "fen|pen", "xu", "xu", "qin", "yi|yin", \
    "wo", "yun", "yuan", "hang|kang", "yan|wei", "chen|shen|tan", "chen", "dan", "you", "dun|zhuan", \
    "hu", "huo", "qi|qie", "mu", "niu|nv", "mei|mo", "ta|da", "mian", "wu|fu|mi", "chong", \
    "pang", "bi", "sha|suo", "zhi", "pei", "pan", "zhui|zi", "za", "gou", "liu", \
    "mei|mo", "ze", "feng", "ou", "li", "guan|lun", "cang", "feng", "gui|wei", "hu", \
    "mei|mo", "mei|hui", "shu", "ju|zu", "za", "tuo|duo", "tuo|duo", "tuo|duo", "he", "zhen|li", \
    "ni|mi", "yi|chi", "fa", "fei|fu", "you", "tian", "chi|yi|zhi", "zhao", "gu", "chan|dian|tian|zhan", \
    "yan", "si", "kuang", "jiong|ying", "gou|ju", "xie|yi", "qiu|you", "die|yi", "jia", "you|zhong", \
    "quan", "bo|po", "hui", "mi|bi", "ben", "ze", "ku|zhu", "le", "ao|you", "gu", \
    "hong", "gan|han", "fa", "liu|mao", "si", "hu", "ping|peng", "ci|zi", "fa|fan|feng", "di|zhi|chi", \
    "su", "ning|zhu", "cheng", "ling", "pao", "bei|bi|bo", "li|qi|se", "si", "ni", "ju", \
    "yue|xue|sa", "zhou|zhu", "sheng", "lei", "juan|xuan", "xue|jue", "fu", "pan", "mian|min", "tai", \
    "yang", "ji", "yong", "guan", "beng|liu|pin", "xue", "long|shuang", "lu", "dan", "luo|po", \
    "xie", "bo|po", "ze", "jing", "yin", "zhou|pan", "ji|jie", "shi|ye", "hui", "hui", \
    "zai", "cheng", "yan|ye|yin", "wei", "hou", "cun|jian", "xiang|yang", "lie", "si", "ji", \
    "er", "xing", "fu", "sa|xian|xun", "zi|se|qi", "zhi", "yin", "wu", "xi|xian", "kao", \
    "zhu", "jiang", "luo", "uu", "an|yan|e", "dong|tong", "yi|ti", "mou|si", "lei", "yi", \
    "mi", "quan", "jin", "po", "wei", "xiao", "xie|yi", "hong", "xu|yi", "su|shuo", \
    "kuang", "dao|tao|yao", "jie|qie", "ju", "er", "zhou", "ru", "ping", "xuan|xun", "xiong", \
    "zhi", "guang|huang", "huan", "ming", "guo|huo", "gui|wa", "he|qia", "pai|pa", "hu|wu", "qu", \
    "liu", "yi", "jia|xia", "jing", "qian|jian", "jiang", "ao|jiao|nao", "cheng|zhen", "shi", "zhuo", \
    "ce", "peol", "hui|kuai", "ji|qi", "liu", "chan", "gun|hun", "hu|xu", "nong", "xun|yin", \
    "jin", "lie", "qiu", "wei", "zhe", "jun|xun", "gan|han", "bang|bin", "mang", "zhuo", \
    "you|di", "xi", "bo", "dou", "huan", "hong", "yi|ya", "pu", "cheng|ying", "lan", \
    "gao|ge|hao", "lang", "han", "li", "geng", "fu", "wu", "li|lian", "chun", "feng|hong", \
    "yi", "yu", "tong", "lao", "hai", "jin|qin", "jia|xia", "chong", "jiong", "mei", \
    "sui|nei", "cheng", "pei", "jian|xian", "lin|qin|sei|shen", "chu|tu|ye", "kun", "ping", "nie", "han", \
    "jing|qing", "xiao", "die|she", "ren|nian", "tu", "yong|chong", "xiao", "dian|xian|yan", "ting", "e", \
    "shu|sou|su", "tun|yun", "juan|xuan|yuan", "cen|qian|zan", "ti", "li", "shui", "si", "lei", "shui", \
    "chao|dao|shou|tao", "du", "lao", "lai", "lan|lian", "wei", "wo|guo", "yun", "huan|hui", "di", \
    "heng", "run", "jian", "zhang", "se", "fu|pou", "guan", "xing", "shou|tao", "shua|shuan", \
    "ya", "chuo", "zhang", "shi|ye", "kong|nang", "wan|yuan|wo", "han", "tuo", "dong", "he", \
    "wo", "ju", "she", "liang", "hun", "ta", "zhuo", "dian", "qie|ji", "de", \
    "juan", "zi", "xi", "xiao", "qi", "gu|hu", "guo|guan", "yan|han", "lin", "chang|tang", \
    "zhou|diao", "peng", "hao", "chang", "chu|shu", "qi|qian", "fang", "chi", "lu", "zhuo|chuo|nao", \
    "ju", "tao", "cong|shuang", "lei|li", "zhe", "peng|ping", "fei", "song", "tian", "pi|pei", \
    "dan|tan|yan", "yu|xu", "ni", "yu", "lu", "gan|han", "mi", "cheng|jing", "ling", "guan|lun", \
    "yan|yao|yin", "cui|zu", "qu", "huai", "yu", "nian|shen", "shen", "hu|biao", "chun|zhun", "hu", \
    "yuan", "lai", "gun|hun|kun", "qing", "yan", "qian|jian", "tian", "miao", "zhi", "yin", \
    "bo|po", "ben", "yuan", "wen|min", "ruo|re|luo", "fei", "qing", "yuan", "he|jie|kai|ke", "ji|qi", \
    "die|she", "yuan", "se", "lu", "qi|se|zi", "du|dou", "qi", "chan|jian|qian", "mian|sheng", "pi", \
    "xi", "yu", "yuan", "shen", "lin|qin|sen|shen", "rou", "huan", "zhu", "jian", "nuan", \
    "yu", "qiu|wu", "ting", "qu|ju", "du", "feng|fan", "zha", "bo", "ou|wo|wu", "wo|guo", \
    "di|ti", "wei", "wen|yun", "er|nuo|ruan", "die|xie|yi|zha", "ce", "wei", "he", "gang|jiang", "yan", \
    "hong|gong", "xuan", "mi", "he|jie|kai|ke", "mao", "ying", "yan", "liu|you", "hong|qing", "miao", \
    "sheng", "mei", "zai", "gun|hun", "nai", "gui", "chi", "e", "ba|pai", "mei", \
    "lan|lian", "qi", "qi", "mei", "tian", "cou", "wei", "can", "tuan|zhuan", "mian", \
    "min|xu|hui", "bo|po", "xu", "ji", "pen", "jian|qian|zan|zhan", "jian", "hu", "feng", "xiang", \
    "yi", "yin", "chen|dan|tan|zhan", "shi", "jie|xie", "zhen|zheng", "huang|kuang", "tan", "yu", "bi", \
    "min|hun", "shi", "tu", "sheng", "yong", "ju", "dong|tong", "tuan|nuan", "qiu|jiao", "qiu|jiu|jiao", \
    "qiu", "yan|yin", "tang|shang", "long", "huo", "yuan", "nan", "pan|ban", "you", "quan", \
    "hun|zhuang", "liang", "chan", "dian|xian|yan", "zhun|chun", "nie|he", "zi", "wan", "shi|ta|xi", "man|men", \
    "ying", "la", "kui|hui", "feng|hong", "jian", "xu", "lou", "wei", "gai|xie", "xia", \
    "ying", "po", "jin", "gui|yan", "tang", "yuan", "suo", "yuan", "xian|lian|nian", "yao", \
    "meng", "zhun|zhuo", "cheng", "kai|ke", "tai", "ta|da", "wa", "liu", "gou|gang", "sao", \
    "mi|ming", "zha", "shi", "yi", "lun", "ma", "bu|fu|po|pu", "wei|mei", "li", "zai", \
    "wu", "xi", "wen", "qiang", "ze", "shi", "shuo|su", "ai", "qin|zhen", "shao|sou", \
    "yun", "chou|xiu", "yin", "rong", "hun", "su", "suo|se", "ni|niao", "ta", "shi", \
    "ru", "ai", "pan", "xu|chu", "chu", "pang|peng", "weng", "cang", "mie", "ge", \
    "dian|tian|zhen", "hao|xue", "huang", "qi|xie|xi", "ci|zi", "di", "zhi", "ying|xing", "fu", "jie", \
    "gu|hua", "ge", "zi", "tao", "teng", "sui", "bi", "jiao", "hui", "gun", \
    "yao|yin", "gao|hao|ze", "long|shuang", "chi|zhi", "yan", "ni|she", "man|men", "ying", "chun", "lu|lv", \
    "jian|lan", "luan", "xiao", "bin", "han|nan|tan", "yu", "xiu", "hu", "bi", "biao", \
    "chi|zhi", "jiang", "kou", "lin|qin|sen|shen", "shang", "di", "mi", "ao", "lu", "hu|xu", \
    "hu", "you", "chan", "fan", "yong", "gun", "man|men", "qing", "yu", "biao|piao", \
    "ji", "ya", "chao", "qi|qie", "xi", "ji", "lu", "lou|lu", "long", "jin", \
    "guo", "cong|song", "lou", "zhi", "gai", "qiang", "li", "yan", "cao", "jiao", \
    "cong", "chun", "tuan|zhuan", "ou", "teng", "ye", "xi", "mi", "tang", "mo", \
    "tang|shang", "han|tan", "lan|lian", "lan", "wa", "chi|tai", "gan", "feng|peng", "xuan", "yi", \
    "man", "qi|se|zi", "mang", "kang", "luo|ta", "peng", "shu", "zhang", "zhang", "chong|zhuang", \
    "xu", "huan", "kuo|huo", "chan|jian|qian", "yan", "shuang|chuang", "liao|xiao", "cui", "ti", "yang", \
    "jiang", "zong|cong", "ying", "hong", "xin", "shu", "guan|huan", "ying", "xiao", "zong|cong", \
    "kun", "xu", "lian", "zhi", "wei", "pie|pi", "jue|yu|shu", "jiao|qiao", "po|bo", "xiang|dang", \
    "hui", "jie", "wu", "pa", "ji", "bo|pan", "wei|gui", "sou|su|xiao", "qian", "qian", \
    "xi|ya", "lu", "xi", "xun|sun", "dun", "huang|guang", "min", "run", "su", "liao|lao", \
    "zhen", "zong|cong", "yi", "zhi|zhe", "wan", "tan|shan", "dan|tan|xun|yin", "chao", "xun|yin", "kui|hui", \
    "ye", "shao", "tu|zha", "zhu", "san|sa", "hei", "bi", "shan", "chan", "chan", \
    "shu", "chong|tong|zhong", "pu", "lin", "wei", "se", "se", "cheng|deng", "jiong", "cheng|deng", \
    "hua", "ao|jiao|nao", "lao", "che", "gan|han", "cun", "hong|jing", "si", "shu|zhu", "peng", \
    "han", "yun", "liu", "hong|gong", "fu", "hao|gao", "he", "xian", "jian", "shan", \
    "xi", "ao|yu", "lu", "lan", "ning", "yu", "lin", "mian|sheng", "cao|zao", "dang", \
    "han|huan", "ze|shi", "xie", "yu", "li", "shi|cuo", "xue|xiao", "ling", "wan|man|ou", "ci|zi", \
    "yong", "hui|kuai", "can", "lian", "dian", "ye", "ao|yu", "huan|xuan", "zhen", "chan|dan|zhan", \
    "man", "gan|dan", "dan|tan", "yi", "sui", "pi", "ju", "ta", "qin", "ji|jiao", \
    "zhuo", "lian|xian", "nong", "guo|wo", "jin|qin", "pen|fen", "se", "ji|sha", "sui", "hui|huo|hun", \
    "chu", "ta", "song", "ding|ting", "se", "zhu", "lai", "bin", "lian", "mi|ni", \
    "shi|ta|xi", "shu", "mi", "ni|ning", "ying", "ying", "meng", "jin", "qi", "bi|pi", \
    "ji|qi", "hao", "er|nuan|ruan", "cui|zui", "wo", "chao|dao|shou|tao", "yin", "yin", "dui", "ci", \
    "huo|hu", "qing|jing", "jian|lan", "jun|xun", "kai|ai", "pu", "zhuo|zhao", "wei", "bin|bang", "gu", \
    "qian", "ying", "bin", "kuo", "fei", "cang", "me", "jian|zan", "wei|dui", "luo|po", \
    "cuan|qian|za|zan", "lu", "li", "you", "yang", "lu", "si", "zhi", "jiong|ying", "du|dou", \
    "wang", "hui", "xie", "pan", "chen|pan|shen", "biao", "chan", "mie|mo", "liu", "jian", \
    "pu|bao", "se", "cheng", "gu", "bin", "huo", "xian", "lu", "qin", "han", \
    "ying", "rong", "li", "jing|cheng", "xiao", "ying", "sui", "wei|dui", "xie", "huai|wai", \
    "xue", "zhu", "long|shuang", "lai", "dui", "fan", "hu", "lai", "shu", "ling|lian", \
    "ying", "mi|ni", "ji", "lian", "jian|zun", "ying", "fen", "lin", "yi", "jian", \
    "yao|yue", "chan", "dai", "rang|nang", "jian", "lan", "fan", "shuang", "yuan", "jiao|ze|zhuo", \
    "feng", "she|ni", "lei", "lan", "cong", "qu", "yong", "qian", "fa", "guan|huan", \
    "jue", "yan", "hao", "ying", "li|sa|xian", "zan|cuan", "luan", "yan", "li", "mi", \
    "shan", "han|nan|tan", "dang|tang", "jiao", "chan", "ying", "hao", "ba", "zhu", "lan", \
    "lan", "nang", "wan", "luan", "quan|xun", "xian", "yan", "gan", "yan", "yu", \
    "huo", "huo", "mie", "guang", "deng|ding", "hui", "xiao", "xiao", "hui", "hong", \
    "ling", "zao", "zhuan", "jiu", "zha|yu", "xie", "chi", "zhuo", "zai", "zai", \
    "can", "yang", "qi", "zhong", "fen|ben", "niu", "jiong|gui", "wen", "pu", "yi", \
    "lu", "chui", "pi", "kai", "pan", "tan|yan", "yan|kai", "feng|pang", "mu", "chao", \
    "liao", "que|gui", "hang|kang", "dun|tun", "guang", "xin", "zhi", "guang", "guang", "wei", \
    "qiang", "bian", "da", "xia", "zheng", "zhu", "ke", "zhao", "fu", "ba", \
    "xie", "xie", "ling", "zhuo|chu", "xuan", "ju", "tan", "pao|bao", "jiong", "pao|fou", \
    "tai", "tai", "bing", "yang", "tong", "qian|shan", "zhu", "zha", "dian", "ha|wei", \
    "shi", "lan|lian", "chi", "huang", "zhou", "hu", "luo|shuo|yue", "lan", "jing|ting", "jiao|yao", \
    "xu", "heng", "quan", "lie", "huan", "yang", "xiu|xiao", "xiu", "xian", "yin", \
    "wu|ya", "zhou", "yao", "shi", "wei", "tong|dong", "xue", "zai", "kai", "hong", \
    "lao|luo", "xia", "chong|zhu", "xuan|hui", "zheng", "po", "yan|yin", "ai|hui", "guang", "che", \
    "hui", "kao", "chen", "fan", "shao", "ye", "hui", "uu", "dang|tang", "jin", \
    "re", "lie", "xi", "fu", "jiong", "che|xie", "pu", "jing|ting", "zhuo", "ting", \
    "wan", "hai", "peng", "lang", "yan|shan", "xu", "feng", "chi", "rong", "hu", \
    "xi", "shu", "he|huo", "hun|xun", "ku|kao", "juan|jue", "xiao", "xi", "yan|yi", "han", \
    "zhuang", "jun|qu", "di", "che|xie", "ji|qi", "wu", "uu", "lv", "han", "yan", \
    "huan", "men", "ju", "tao|dao", "bei", "fen", "lin", "kun", "hun", "tun|tui", \
    "xi", "cui", "wu|mo", "hong", "chao|ju", "fu", "wo|ai", "jiao|qiao", "cong", "feng", \
    "ping", "qiong", "ruo", "xi|yi", "qiong", "xin", "zhuo|chao", "yan", "yan|yi", "yi", \
    "jiao|qiao", "yu", "gang", "ran", "pi", "ying|xiong|gu", "wang", "sheng", "chang|gua", "shao", \
    "ying|xiong", "nian|ne", "geng", "qu|kuo", "chen", "he", "kui", "zhong", "duan", "xia", \
    "hui|yun|xun", "feng", "lian|lan", "xuan", "xing", "huang", "jiao", "jian", "bi", "ying", \
    "zhu", "wei|hui", "tuan", "shan|qian", "xi|yi", "nuan|xuan", "nuan", "chan", "yan", "jiong", \
    "jiong", "yu", "mei", "sha", "wei", "ye|zha", "jin", "qiong", "rou", "mei", \
    "huan", "xiu|xu", "zhao", "wei|yu", "fan", "qiu", "sui", "yang", "lie", "zhu", \
    "jie", "sao|zao", "gua", "bao", "hu", "wen|yun", "nan", "shi", "liang|huo", "bian", \
    "gou", "tui", "tang", "chao", "shan", "en|yun", "bo", "huang|ye", "xie", "xi", \
    "wu", "xi", "yun", "he", "he|xiao", "xi|yi", "yun", "xiong", "xiong|nai", "shan", \
    "qiong", "yao", "xun", "mi", "qian|lian", "ying|xing", "wu", "rong", "gong", "yan", \
    "qiang", "liu", "xi|yi", "bi", "biao", "cong|zong", "lu|ao", "jian", "shu|shou", "yi", \
    "lou", "feng|peng", "cui|sui", "yi", "tong|teng", "jue", "zong", "yun|yu", "hu", "yi", \
    "zhi", "ao", "wei", "liu", "han|ran", "ou", "re", "jiong", "man", "kun", \
    "shang", "cuan", "zeng", "jian", "xi", "xi", "xi", "yi", "xiao", "chi", \
    "huang", "dan|chan", "ye", "tan|xun", "ran", "yan", "xun", "qiao|xiao", "jun", "deng", \
    "dun|tun", "shen", "jiao|qiao|jue|zhuo", "fen|ben", "si|xi", "liao", "yu", "lin", "tong|dong|jiong", "shao", \
    "fen", "fan|fen", "yan", "xun", "lan", "mei", "tang|dang", "yi", "jiong", "men", \
    "jing", "uu", "cuo|ying", "yu", "yi", "xue", "lan", "lie|tai", "sao|zao", "can", \
    "sui", "xi", "que", "cong|zong", "lian|qian", "hui", "kuo|zhu", "xie", "ling", "yu|wei", \
    "yi", "xie", "zhao", "hui", "da", "nung", "bing", "ru|ruan", "bing|xian", "xiao|he", \
    "xun", "jin", "chou", "tao|dao", "yao|shuo", "he", "lan", "biao", "rong", "li|lie", \
    "mo", "bao|bo", "ruo", "lv", "la|lie", "ao", "xun", "huang|kuang", "shuo|luo", "liao", \
    "li", "lu", "jue", "liao", "yan|xun", "xi", "xie", "long", "ye", "can", \
    "rang", "yue", "lan", "cong", "jue", "chong|tong", "guan", "qu|ju", "che", "mi", \
    "tang", "lan", "kuo|zhu", "lan", "ling", "cuan", "yu", "zhua|zhao", "zhua", "pa", \
    "zheng", "pao", "cheng|chen", "yuan", "ai", "wei", "han", "jue", "jue", "fu", \
    "ye", "ba", "die", "ye", "xiao|yao", "zu", "shuang", "er|mi", "pan|qiang", "chuang", \
    "ke", "zang", "die", "qiang", "yong", "qiang", "pan|pian", "ban", "pan", "chao", \
    "jian", "pai", "du", "chuang", "yu", "zha", "bian|mian", "die", "bang|pang", "bo", \
    "chuang", "you", "yong|you", "du", "ya", "cheng", "niu", "niu", "pin", "le|jiu", \
    "mou|mu", "ta|tuo", "mu", "lao|lou", "ren", "mang", "fang", "mao", "mu", "gang", \
    "wu", "yan", "ge|qiu", "bei", "si", "jian", "gu", "you|chou", "ke|ge", "sheng", \
    "mu", "di|zhai", "qian", "quan", "quan", "zi", "te", "suo|xi", "mang", "keng", \
    "qian", "wu", "gu", "xi", "li", "li", "pou", "ji|yi", "gang", "zhi|te", \
    "ben", "quan", "chun", "du", "ju", "jia", "jian|qian", "feng", "pian", "ke", \
    "ju", "kao", "chu", "xi", "bei", "luo", "jie", "ma", "san", "wei", \
    "li|mao", "dun", "tong", "qiao", "jiang", "xi", "li", "du", "lie", "bai|pai", \
    "piao|pao", "bao|bo", "xi|suo", "chou", "wei", "kui|rao", "chou", "quan", "quan", "ba|quan", \
    "fan", "qiu", "ji", "chai", "bao|zhuo", "han|an", "ge|he", "zhuang", "guang", "ma", \
    "you", "kang|gang", "fei|pei|bo", "hou", "ya", "yin", "huan|fan", "zhuang", "yun", "jue|kuang", \
    "niu|nv", "di|ti", "kuang", "zhong", "mu", "bei", "pi", "ju", "quan|yi|chi", "sheng|xing", \
    "pao", "xia", "yi|tuo", "hu", "ling", "fei", "pi", "ni", "yao", "you", \
    "gou", "xue", "ju", "dan", "bo", "ku", "xian", "ning", "huan|xuan", "hen|ken|yan", \
    "jiao|xiao", "he|mo", "zhao", "ji|jie", "xun", "shan", "shi|ta", "rong", "shou", "tong", \
    "lao|dong", "du", "xia", "shi", "kuai", "zheng", "yu", "sun", "yu", "bi", \
    "mang|zhuo", "xi|shi", "juan", "li", "xia", "yin", "jun|suan", "hang|lang", "bei", "zhi", \
    "yan", "sha", "li", "han", "xian", "jing", "pai", "fei", "xiao", "pi|bai", \
    "qi", "ni", "biao", "yin", "lai", "lie|que|xi", "jian", "qiang", "kun", "yan", \
    "guo|luo", "zong", "mi", "chang", "ji|wei|yi", "zhi", "zheng", "wei|ya", "meng", "cai", \
    "cu", "she", "lie", "ceon", "luo", "hu", "zong", "fui", "wei", "feng", \
    "wo", "yuan", "xing", "zhu", "mao|miao", "wei", "chuan|shan", "xian", "tuan", "jia|ya", \
    "nao", "xie|ge", "jia", "hou", "bian|pian", "you|yao", "you", "mei", "cha|zha", "yao", \
    "sun", "bo|po", "ming", "hua", "yuan", "sou", "ma", "yuan", "dai", "yu", \
    "shi", "hao", "qiang", "yi", "zhen", "cang", "gao|hao", "man", "jing", "jiang", \
    "mao|mu", "zhang", "chan", "ao", "ao", "gao|hao", "cui|suo", "ben|fen", "jue", "bi", \
    "bi", "huang", "pu|bu", "lin", "xu|yu", "tong|zhuang", "xiao|yao", "lao|liao", "que|xi|shuo", "xiao", \
    "shou", "dun|du", "jiao", "ge|lie|xie", "xuan|juan", "du", "hui", "kuai|hua", "xian", "ha|jie|xie", \
    "ta", "xian", "mi|xun", "ning", "bian|pian", "huo", "ru|nou", "meng", "lie", "you|nao", \
    "guang|jing", "shou", "lu", "ta", "xian|suo", "mi", "rang", "huan|quan", "nao", "e|luo", \
    "xian", "qi", "jue", "xuan", "miao|yao", "zi|ci|xuan", "shuai|lv", "lu", "yu", "su", \
    "wang|yu", "qiu", "ga", "ding", "le", "ba", "ji", "hong", "di", "chuan", \
    "gan", "jiu", "yu", "qi", "yu", "yang|chang", "ma", "hong", "wu", "fu", \
    "wen|min", "jie", "ya", "fen|bin", "men", "bang", "yue", "jue", "yun|men", "jue", \
    "wan", "yin|jian", "mei", "dan", "pin", "wei", "huan", "xian", "qiang", "ling", \
    "dai", "yi", "gan|an", "ping", "dian", "fu", "xian|xuan", "xi", "bo", "ci|cou", \
    "gou", "jia", "shao", "po", "ci", "ke", "ran", "sheng", "shen", "yi|tai", \
    "zu|ju", "jia|ka", "min", "shan", "liu", "bi", "zhen", "zhen", "jue", "fa", \
    "long", "jin", "jiao", "jian", "li", "guang", "xian", "zhou", "gong", "yan", \
    "xiu", "yang", "xu", "li|luo", "su", "zhu", "qin", "yin|ken", "xun", "bao", \
    "er", "xiang", "yao", "xia", "hang|heng", "gui", "chong", "xu", "ban", "pei", \
    "lao", "dang", "ying", "hui|hun", "wen", "e", "cheng|ting", "di|ti", "wu", "wu", \
    "cheng", "jun", "mei", "bei", "ting", "xian", "chu", "han", "xuan|qiong", "yan", \
    "qiu", "xuan", "lang", "li", "xiu", "fu", "liu", "ya", "xi", "ling", \
    "li", "jin", "lian", "suo", "suo", "feng", "wan", "dian", "bing|pin", "zhan", \
    "cui|se", "min", "yu", "ju", "chen", "lai", "min", "sheng|wang", "wei|yu", "tian", \
    "chu", "zhuo|zuo", "pei|beng", "cheng", "hu", "qi", "e", "kun", "chang", "qi", \
    "beng", "wan", "lu", "cong", "guan|gun", "yan", "diao", "bei|fei|pei", "lin", "qin", \
    "pi", "pa", "qiang", "zhuo", "qin", "fa", "jin", "qiong", "du", "jie", \
    "hui|hun", "yu", "mao|q", "mei", "chun", "xuan", "ti", "xing", "dai", "rou", \
    "min", "jian", "wei", "ruan", "huan", "jie|xie", "chuan", "jian", "zhuan", "yang|chang", \
    "lian", "quan", "xia", "duan", "huan|yuan", "ye|ya", "nao", "hu", "ying", "yu", \
    "huang", "rui", "se", "liu", "shi", "rong", "suo", "yao", "wen", "wu", \
    "zhen", "jin", "ying", "ma", "tao", "liu", "tang", "li", "lang", "gui", \
    "tian|zhen", "cang|qiang|cheng", "cuo", "jue", "zhao", "yao", "ai", "bin|pian", "tu|shu", "chang", \
    "kun", "zhuan", "cong", "jin", "yi", "cui", "cong", "ji|qi", "li", "jing", \
    "zao|suo", "qiu", "xuan", "ao", "lian", "men", "zhang", "yin", "hua|ye", "ying", \
    "wei", "lu", "wu", "deng", "xiu", "zeng", "xun", "qu", "dang", "lin", \
    "liao", "jue|qiong", "su", "huang", "gui", "pu", "jing", "fan", "jin", "liu", \
    "ji", "hui", "jing", "ai", "bi", "can", "qu", "zao", "dang", "jiao", \
    "gun|guan", "tan", "kuai|hui", "huan", "se", "sui", "tian", "chu", "yu", "jin", \
    "fu|lu", "bin|pian", "shu", "wen", "zui", "lan", "xi", "ji|zi", "xuan", "ruan", \
    "wo", "gai", "lei", "du", "li", "zhi", "rou", "li", "zan", "qiong|xuan", \
    "ti", "gui", "sui", "la", "long", "lu", "li", "zan", "lan", "ying", \
    "mi|xi", "xiang", "qiong|wei", "guan", "dao", "zan", "huan|ye|ya", "gua", "bo", "die", \
    "pao|bo", "gu|hu|huo", "hu|zhi", "piao", "ban", "rang", "li", "wa", "shiwa", "hong|xiang", \
    "qianwa", "ban", "pen", "fang", "dan", "weng", "ou", "fenwa", "miliklanm", "wa", \
    "hu", "ling", "yi", "ping", "ci", "baiwa", "juan", "chang", "chi", "liwa", \
    "dang", "wa|meng", "bu", "zhui", "ping", "bian", "zhou", "juan|zhen", "liwa", "ci", \
    "ying", "qi", "xian", "lou", "di", "ou", "meng", "chuan|zhuan", "beng", "lin", \
    "zeng", "wu", "pi", "dan", "weng", "ying", "yan", "gan|han", "dai", "shen", \
    "tian", "tian", "han", "chang", "sheng", "qing", "shen", "chan", "chan", "rui", \
    "sheng", "su", "shen", "yong", "shuai", "lu", "fu|pu", "dong|yong", "beng|qi", "beng|feng", \
    "ning", "tian", "yao|you", "jia", "shen", "you|zha", "dian", "fu", "nan", "dian|sheng|tian|ying", \
    "ping", "ting|ding", "hua", "ding", "quan|zhen", "zi|zai", "meng|mang", "bi", "bi", "jiu|liu", \
    "sun", "liu", "chang", "mu", "tian|yun", "fan", "fu", "geng", "tian", "jie", \
    "jie", "quan", "wei", "fu|bi", "tian", "mu", "tap", "pan", "jiang", "wa", \
    "fu|da", "nan", "liu", "ben", "zhen", "chu|xu", "mu", "mu", "ji|ce", "zi|zai|tian", \
    "gai", "bi", "da", "zhi|shi|chou", "lue", "qi", "lue", "fan|pan", "yi", "fan|pan", \
    "hua", "she|yu", "she|yu", "mu", "jun", "yi", "liu", "she", "die", "chou", \
    "hua", "dang", "zhui", "ji|qi", "wan", "jiang", "cheng", "chang", "tun|tuan", "lei", \
    "ji", "cha|chai", "liu", "die", "tuan", "lin", "jiang", "jiang", "chou", "pi", \
    "die", "die", "pi|ya|shu", "jie|qie", "dan", "shu", "shu", "di|zhi", "ning|yi", "ne", \
    "nai", "ding|ne", "bi", "jie", "liao", "gang", "ge|yi", "jiu", "zhou", "xia", \
    "shan", "xu", "nue|yao", "li|lai", "yang", "chen", "you", "ba", "jie", "jue|xue", \
    "qi", "xia|ya", "cui", "bi", "yi", "li", "zong", "chuang", "feng", "zhu", \
    "pao", "pi", "gan", "ke|qia", "ci|ji|zhai|zi", "xue", "zhi", "dan|da", "chen|zhen", "bian|fa", \
    "zhi", "teng", "ju", "ji", "fei", "ju", "shan", "jia", "xuan", "zha", \
    "bing", "nei|ni", "zheng", "yong", "jing", "quan", "chong|teng", "tong", "yi", "jie", \
    "you|wei", "hui", "shi|tan", "yang", "chi", "zhi", "gen|hen", "ya", "mei", "dou", \
    "jing", "xiao", "tong", "tu", "mang", "pi", "xiao", "suan", "pu", "li", \
    "zhi", "cuo", "duo", "pi|wu", "sha", "lao", "shou", "huan", "xian", "yi", \
    "peng|beng", "zhang", "guan", "tan", "fei", "ma", "ma|lin", "chi", "ji", "dian|tian", \
    "an|ye", "chi", "bi", "bi|pi", "min", "gu", "dui", "ke", "wei", "yu", \
    "cui", "ya", "zhu", "cu", "dan", "shen", "zhong", "chi|zhi", "yu", "hou", \
    "feng", "la", "dang|yang", "chen", "tu", "yu", "guo", "wen", "huan", "ku", \
    "xia|jia", "yin", "yi", "lou", "sao", "jue", "chi", "xi", "guan", "yi", \
    "wen|wo|yun", "ji", "chuang", "ban", "hui|lei", "liu", "chai|cuo", "shou", "nue|yao", "dian|chen", \
    "da", "bie", "tan", "zhang", "biao", "shen", "cu", "luo", "yi", "zong", \
    "chou|lu", "zhang", "ji|zhai", "sou", "se", "que", "diao", "lou", "lou|lv", "mo", \
    "qin", "yin", "ying", "huang", "fu", "liao|shuo", "long", "jiao|qiao", "liu", "lao", \
    "xian", "fei", "dan|tan", "yin", "he", "ai", "ban", "xian", "guan", "wei|gui", \
    "nong", "yu", "wei", "yi", "yong", "pi", "lei", "li", "shu", "dan", \
    "lin|bing", "dian", "lin|bing", "lai|la", "bie", "ji", "chi", "yang", "xuan", "jie", \
    "zheng", "me", "li", "huo", "lai|la", "ji", "dian", "xuan", "ying", "yin", \
    "qu", "yong", "tan", "dian", "luo", "luan", "luan", "bo", "uu", "gui", \
    "ba", "fa", "de|deng", "fa|bo", "bai|bo", "bai|bo|mo", "qie", "bi|ji", "zao", "zao", \
    "mao", "de|di", "ba|pa", "jie", "huang|wang", "gui", "ci", "ling", "gao|yao", "mo", \
    "ji", "jiao", "peng", "gao|yao", "ai", "e", "hao|hui", "han", "bi", "huan|wan", \
    "chou", "qian", "xi", "ai", "po|xiao", "hao", "huang", "hao", "ze", "cui", \
    "hao", "xiao", "ye", "pan|po", "hao", "jiao", "ai", "xing", "huang", "li|luo|bo", \
    "piao", "he", "jiao", "pi", "gan", "pao", "zhou", "jun", "qiu", "cun", \
    "que", "zha", "gu", "jun", "jun", "zhou", "cu|zha", "uu", "zhao|zhan|dan", "du", \
    "min", "qi", "ying", "yu", "bei", "zhao", "chong|zhong", "pen", "he", "ying", \
    "he", "yi", "bo", "wan", "he|ke", "ang", "zhan", "yan", "jian", "an|he", \
    "yu", "kui", "fan", "gai|ge", "dao", "pan", "fu", "qiu", "sheng|cheng", "dao", \
    "lu", "zhan", "meng|ming", "li", "jin", "xu", "jian|kan", "pan|xuan", "guan", "an", \
    "lei|lu|lv", "xu", "chou|zhou", "dang", "an", "gu", "li", "mu", "cheng|ding", "gan", \
    "xu", "mang", "mang|wang", "zhi", "qi", "yuan", "tian|xian", "xiang", "dun|zhun", "xin", \
    "pan|xi", "fen|pan", "feng", "dun|yun", "min", "ming", "sheng|xing", "shi", "hun|yun", "mian", \
    "pan", "fang", "miao", "chen|dan", "mei", "mao|mei", "kan", "xian", "kou", "shi", \
    "yang|ying", "zheng", "ao|yao", "shen", "huo", "da", "zhen", "kuang", "xu|ju", "shen", \
    "yi|chi", "sheng", "mei", "mo|mie", "zhu", "zhen", "zhen", "mian|min", "shi", "yuan", \
    "die|chou", "ni", "zi", "zi", "chao", "zha", "huan|juan|xuan", "bing|fang", "pan|mi", "long", \
    "gui|sui", "tong", "mi", "zhi|die", "di", "ne", "ming", "xun|shun|xuan", "chi", "kuang", \
    "juan", "mou", "zhen", "tiao", "yang", "wen|yan", "mo|mi", "zhong", "mo", "zhe|zhao|zhuo", \
    "zheng", "mei", "juan|suo", "shao|xiao|qiao", "han", "huan", "di|ti", "cheng", "cuo|zhuai", "juan", \
    "e", "man", "xian", "xi", "kun", "lai", "jian", "shan", "tian", "gun|huan", \
    "wan", "leng", "shi", "qiong", "li|lie", "ya", "jing", "zheng", "li", "lai", \
    "sui|zui", "juan", "shui", "hui|sui", "du", "pi", "bi|pi", "mu", "hun", "ni", \
    "lu", "gao|yi", "jie|she", "cai", "zhou", "yu", "hun", "ma", "xia", "xing", \
    "hui", "gun", "zai", "chun", "jian", "mei", "du", "hou", "xuan", "tian", \
    "ji|kui", "gao|hao", "rui", "mao|wu", "xu", "fa", "wo", "miao", "chou", "kui|gui", \
    "mi", "weng", "kou|ji", "dang", "chen|shen|tian", "ke", "sou", "xia", "huan|qiong", "mo", \
    "meng|mian|ming", "man", "fen", "ze", "zhang", "yi", "diao|dou", "kou", "mo", "shun", \
    "cong", "lou|lv", "chi", "man|men", "piao", "cheng|zheng", "gui", "mang|meng", "huan|wan", "shun", \
    "bi|pie", "xi", "qiao", "pu", "zhu", "deng", "shen", "shun", "liao", "che", \
    "jian|xian", "kan", "ye", "xu|xue", "tong", "wu|mou|mi", "lian|lin", "gui|kui", "jian|xian", "ye", \
    "ai", "hui", "zhan", "jian", "gu", "zhao", "ju|qu", "wei|mei", "chou", "sao", \
    "ning|cheng", "xun", "yao", "huo|yue", "meng", "mian", "pin", "mian", "lei", "kuang|guo", \
    "jue", "xuan", "mian", "huo", "lu", "meng", "long", "guan|quan", "man", "li|xi", \
    "chu", "tang", "kan", "zhu", "mao", "jin|qin|guan", "jin|qin|guan", "yu|xu|jue", "shuo", "ze|zhuo", \
    "jue", "shi", "xian|yi", "shen", "zhi", "hou", "shen", "ying", "ju", "zhou", \
    "jiao", "cuo", "duan", "ai", "jiao", "zeng", "yue", "ba", "shi|dan", "ding", \
    "qi|diao", "ji", "zi", "gan|han", "wu", "da|zhe", "ku|qia", "gang|kong", "xi", "fan", \
    "kuang", "dang", "ma", "sha", "dan", "jue", "li", "fu", "min|wen", "e", \
    "hua|xu", "kang", "zhi", "qi|qie", "kan", "jie", "fen|pin", "e", "ya", "pi", \
    "zhe", "xing|yan", "sui", "zhuan", "che", "dun", "wa", "yan", "jin", "feng", \
    "fa|ge|jie", "mo", "zha|zuo", "zu|ju", "yu", "ke|luo", "tuo", "tuo", "di", "zhai", \
    "zhen", "e", "fei|fu", "mu", "zhu", "la|li", "bian", "nu", "ping", "peng|ping", \
    "ling", "bao|pao|pu", "le", "po", "bo|e", "po", "shen", "za", "ai", "li", \
    "long", "tong", "yong", "li", "kuang", "chu", "keng", "quan", "zhu", "guang|kuang", \
    "gui|he", "e", "nao", "qia", "lu", "wei|gui", "ai", "ge|luo", "ken|yin|xian", "keng|xing", \
    "xing|yan", "dong|tong", "peng|ping", "xi", "lao", "hong", "shuo", "xia", "qiao", "qing", \
    "ai|wei", "qiao", "ji|ce", "keng|qing", "qiao|xiao", "ku|ke|que", "chan", "lang", "hong", "yu", \
    "xiao", "xia", "mang|bang", "long|luo", "yong|tong", "che", "che", "e|wo|yi", "chu|liu", "geng|ying", \
    "mang", "que", "yan", "sha", "kun", "yu|gu", "ceok", "hua", "lu", "cen|chen", \
    "jian", "nue", "song", "zhuo", "keng", "peng", "yan", "chui|zhui|duo", "kong", "cheng", \
    "qi", "zong|cong", "qing", "lin", "jun", "bo|pan", "ding", "min", "diao", "zhan|jian", \
    "he", "lu|liu", "ai", "sui", "xi|que", "leng", "bei", "yin", "dui", "wu", \
    "qi", "lun", "wan", "dian", "gang|nao", "bei", "qi", "chen", "ruan", "yan", \
    "die|she", "ding", "zhou", "tuo", "jie|ya", "ying", "bian", "ke", "bi", "wei", \
    "shuo", "zhen|an", "duan", "xia", "dang", "ti|di", "nao", "peng", "jian|xian", "di", \
    "tan", "cha", "tian", "qi", "dun", "feng", "xuan", "que", "que|qiao", "ma", \
    "gong", "nian", "xie|su", "e", "ci", "liu", "ti|si", "tang", "bang|pang", "ke|hua", \
    "pi", "kui|wei", "sang", "lei", "cuo", "tian", "xia|qia", "qi", "lian", "pan", \
    "ai|wei", "yun", "chui|dui", "zhe", "ke", "la", "pak", "yao", "gun", "tuan|tuo|zhuan", \
    "chan", "qi", "ao|qiao", "peng", "liu", "lu", "kan", "chuang", "ca|chen", "yin", \
    "lei", "piao", "qi", "mo", "qi|zhu", "cui", "zong", "qing", "chuo", "lun", \
    "ji", "shan", "lao|luo", "qu", "zeng", "deng", "jian", "xi", "lin|ling", "ding", \
    "dian", "huang|kuang", "pan|bo", "ji|she|za", "ao|qiao", "di", "li", "jian", "jiao", "xi", \
    "zhang", "qiao", "dun", "jian|xian", "yu", "zhui", "he|qiao", "huo|ke", "ze", "lei", \
    "jie", "chu", "ye", "que|hu", "dang", "yi", "jiang", "pi", "pi", "yu", \
    "pin", "e|qi", "ai|yi", "ke", "jian", "yu", "ruan", "meng", "pao", "ci", \
    "bo", "yang", "ma", "ca", "xian|xin", "kuang", "lei", "lei", "zhi", "li", \
    "li|luo", "fan", "que", "pao", "ying", "li", "long", "long", "mo", "bo", \
    "shuang", "guan", "lan|jian", "ca", "yan", "qi|shi|zhi", "pianpang|shi", "li", "reng", "she", \
    "yue", "si", "qi|zhi", "ta", "ma", "xie", "yao", "xian", "zhi|qi|chi", "gui|qi", \
    "zhi", "fang|beng", "dui", "chong|zhong", "uu", "yi", "shi", "you", "zhi", "tiao", \
    "fei|fu", "fu", "mi", "jie|zu", "qi|zhi", "suan", "mei", "zuo", "qu", "hu", \
    "chu|zhou|zhu", "shen", "sui", "ci|si", "chai", "mi|ni", "lv", "yu", "xiang", "wu", \
    "tiao", "piao", "zhu", "gui", "xia", "zhi", "ji|zhai", "gao", "zhen", "gao", \
    "shui|lei", "jin", "shen", "gai", "kun", "di", "dao", "huo", "tao", "qi", \
    "gu", "guan", "zui", "ling", "lu", "bing", "jin", "dao", "zhi", "lu", \
    "chan|shan", "bi|pi", "chu|zhe", "hui", "you|chao", "xi", "yin", "zi", "huo", "zhen", \
    "fu", "yuan", "xu|wu", "xian", "yang|shang", "zhi|ti", "yi", "mei", "si", "di", \
    "bei", "zhuo", "zhen", "yong", "ji", "gao", "tang", "si", "ma", "ta", \
    "fu", "xuan", "qi", "yu", "xi", "ji|qi", "si", "chan|shan", "dan", "gui", \
    "sui", "li", "nong", "mi|ni", "dao", "li", "rang", "yue", "ti|zhi", "zan", \
    "lei", "rou", "yu", "yu|ou", "chi|li", "xie", "qin", "he", "tu", "xiu", \
    "si", "ren", "tu", "zi", "na|cha", "gan", "zhi|yi", "xian", "bing", "nian", \
    "qiu", "qiu", "zhong|chong", "fen", "mao|hao", "yun", "ke", "miao", "zhi", "jing", \
    "bi", "zhi", "yu", "mi|bi", "ku", "ban", "pi", "ni", "li", "you", \
    "ju|zu", "pi", "bo", "ling", "mo", "cheng|ping", "nian", "qin", "yang", "zuo", \
    "zhi", "zhi", "shu", "ju", "zi", "kuo|huo", "ji|zhi", "cheng|chen", "tong", "zhi|shi", \
    "kuo|huo", "he|huo|ge", "yin", "zi", "zhi", "ji|jie", "ren", "du", "chi|yi", "zhu", \
    "hui", "nong", "bu|pu|fu", "xi", "gao", "lang", "fu", "ze|xun", "shui|tuan|tui|tuo", "lv", \
    "kun", "gan", "jing", "ti", "cheng", "tu|shu", "shao", "shui|tuan|tui|tuo", "ya", "lun", \
    "lu", "gu", "zuo", "ren", "zhun", "bang", "bai", "qi|ji", "zhi", "zhi", \
    "kun", "ling|leng", "peng", "hua|ke", "bing|lin", "chou|diao|tiao", "zu|zui", "yu", "su", "lue|su", \
    "uu", "yi", "qie|xi", "bian", "ji", "fu", "bi|pi", "nuo", "jie", "chong|zhong", \
    "zong", "xu", "cheng|chen", "dao", "wen", "xian|lian", "zi|jiu", "yu", "ji|ze", "xu", \
    "bian|zhen", "zhi", "dao", "jia", "ji|qi", "gao|kao", "gao", "gu|yu", "rong", "sui", \
    "rong", "ji", "kang", "mu", "can|cen|shan", "mi|men", "ti|zhi", "ji", "lu|jiu", "su", \
    "ji", "ying", "wen", "qiu", "se", "kweok", "yi", "huang", "qie", "ji", \
    "sui", "rao|xiao", "pu", "jiao", "bo|zhuo", "zhong|tong", "zui", "lu|lv", "sui", "nong", \
    "se", "hui", "rang", "nuo", "yu", "pin", "ji|zi", "tui", "wen", "chen|cheng", \
    "huo|hu", "kuang", "lv", "biao|pao", "se", "rang|reng", "zhuo|jue", "li", "zan|cuan", "jue|xue", \
    "wa|ya", "jiu", "qiong", "xi", "kong|qiong", "kong", "yu", "shen", "jing", "yao", \
    "chuan|yuan", "tun|zhun", "tu", "lao", "qie", "zhai", "yao", "bian", "bao", "yao", \
    "bing", "wa", "ku|zhu", "jiao|liao|liu|pao", "qiao", "diao", "wu", "wa|gui", "yao", "die|zhi", \
    "chuang", "yao", "tiao|yao", "jiao|zao", "chuang|cong", "jiong", "xiao", "cheng", "kou", "cuan", \
    "wo", "dan", "ku", "ke", "zhuo", "huo|xu", "su", "guan", "kui", "dou", \
    "zhuo", "yin|xun", "wo", "wa", "ya|ye", "dou|yu", "lou|ju", "qiong", "qiao|yao", "yao", \
    "tiao", "chao", "yu", "tian", "diao", "lou|ju", "liao", "xi", "wu", "kui", \
    "chuang", "ke|zhao", "kuan", "cuan|kuan", "long", "cheng", "cui", "liao", "zao", "cuan", \
    "qiao", "qiong", "dou|du", "zao", "long", "qie", "li|wei", "chu", "shi", "fu", \
    "qian", "chu", "hong", "qi", "hao", "sheng", "fen", "shu", "miao", "qu|kou", \
    "zhan", "zhu", "ling", "long|neng", "bing", "jing", "jing", "zhang", "bai", "si", \
    "jun", "hong", "tong|zhong", "song", "zhen|jing", "diao", "yi", "shu", "jing", "qu", \
    "jie", "ping", "duan", "li", "zhuan", "ceng", "deng", "cun", "wai", "jing", \
    "kan", "jing", "zhu", "du|zhu", "le|jin", "peng", "yu", "chi", "gan", "mang", \
    "du|zhu", "wan", "du", "ji", "jiao", "ba", "suan", "ji", "qin", "zhao", \
    "sun", "ya", "rui|zhui", "yuan", "hu|wen|wu", "hang", "xiao", "cen|jin|han", "bi|pi", "bi", \
    "jian|xian", "yi", "dong", "shan", "sheng", "xia|na|da", "di", "zhu", "na", "chi", \
    "gu", "li", "qie", "min", "bao", "shao|tiao", "si", "fu", "ce|shan", "ben", \
    "ba|bo|fa|pei", "da", "zi", "di", "ling", "ze|zuo", "nu", "fu|fei", "gou", "fan", \
    "jia", "gan", "fan", "shi", "mao", "po", "shi|xiao", "jian", "qiong", "long", \
    "min", "bian", "luo", "gui", "qu", "chi", "yin", "yao", "xian", "bi", \
    "qiong", "kuo", "deng", "jiao", "jin|qian", "quan", "sun|xun|yun", "ru", "fa", "kuang", \
    "zhu", "dong|tong", "ji", "da", "hang", "ce", "zhong", "kou", "lai", "bi", \
    "shai|shi", "dang", "zheng", "ce", "fu", "yun|jun", "tu", "pa", "li", "lang", \
    "ju", "guan", "jian|xian", "han", "tong|yong", "xia", "zhi", "cheng", "suan", "shi", \
    "zhu", "zuo", "xiao", "shao", "ting", "jia|ce", "yan", "gao", "kuai", "gan", \
    "chou|tao", "kuang", "gang", "xun|yun", "o", "qian", "xiao", "jian", "bu|pou|fu", "lai", \
    "bei|bi|zou", "bei|pai", "bi", "bi|pi", "ge", "tai|chi", "guai|dai", "yu", "jian", "dao|zhao", \
    "gu", "hu|chi", "zheng", "jing|qing", "sha|zha", "zhou", "lu", "bo", "ji", "lin", \
    "suan", "jun|qun", "fu", "zha", "gu", "kong", "qian", "quan|qian", "jun", "chui|zhui", \
    "guan", "wan|yuan", "ce", "zu", "bo|po", "ze|zhai", "qie", "tuo", "luo", "dan", \
    "xiao", "na|ruo", "jian", "xuan", "bian", "sun", "xiang", "xian", "ping", "jian|zhen", \
    "sheng|xing", "hu", "shi|yi", "zhu|zhuo", "yue|yao|chuo", "chun", "lv", "wu", "dong", "xiao|qiao|shuo", \
    "ji", "jie", "huang", "xing", "mei", "fan", "chuan|duan", "zhuan", "pian", "feng", \
    "zhu", "hong", "qie", "hou", "qiu", "miao", "qian", "gu", "kui", "shi", \
    "ju|lou|lv", "xun|yun", "he", "tang", "yue", "chou", "gao", "fei", "na|ruo", "zheng", \
    "gou", "nie", "qian", "xiao", "cuan", "gong|gan", "pang|peng", "du", "li", "bi|pi", \
    "huo|zhuo", "chu", "shai|shi", "chi", "zhu", "cang|qiang", "long", "lan", "jian", "bu", \
    "li", "hui", "bi", "zhu|di", "cong", "yan", "peng", "sen|zan", "zuan|zhuan", "pi", \
    "biao|piao", "dou", "yu", "mie", "zhuan|tuan", "ze|zhai", "shai", "guo|gui", "yi", "hu", \
    "chan", "kou", "chuo|cou|cu", "ping", "zao", "ji", "gui", "su", "ju|lou|lv", "ce|ji", \
    "lu", "nian", "suo|sui", "cuan", "diao", "suo", "le", "duan", "liang", "xiao", \
    "bo", "mie|mi", "si|shai", "tang|dang", "liao", "dan", "dian", "fu", "jian", "min", \
    "kui", "dai", "jiao", "deng", "huang", "zhuan|sun", "lao", "zan", "xiao", "lu", \
    "shi", "zan", "qi", "pai", "qi", "pi", "gan", "ju", "lu", "lu", \
    "yan", "bo", "dang", "sai", "ke|zhua", "gou", "qian", "lian", "bao|bo|bu", "zhou", \
    "lai", "shi", "lan", "kui", "yu", "yue", "hao", "zhen|jian", "tai", "ti", \
    "mi|nie", "chou|tao", "ji|jie", "yi", "qi", "teng", "zhuan", "zhou", "fan|ban", "shu|sou", \
    "zhou", "qian", "zhuo", "teng", "lu", "lu", "jian", "tuo", "ying", "yu", \
    "lai", "long", "qie", "lian", "lan", "qian", "yue", "zhong", "ju|qu", "lian", \
    "bian", "duan", "zuan", "li", "shi|shai", "luo", "ying", "yue", "zhuo", "xu|yu", \
    "mi", "di|za", "fan", "shen", "zhe", "shen", "nv", "he", "lei", "xian", \
    "zi", "ni", "cun", "zhang", "qian", "zhai", "bi|pi", "ban", "wu", "chao|sha", \
    "kang|jing", "rou", "fen", "bi", "cui|sui", "yin", "zhe", "mi", "tai", "hu", \
    "ba", "li", "gan", "ju", "po", "mo|yu", "cu", "zhan|nian", "zhou", "chi|li", \
    "su", "diao|tiao", "li", "xi", "su", "hong", "tong", "ci|zi", "ce|se", "yue", \
    "zhou|yu", "lin", "zhuang", "bai", "lao", "fen", "er", "qu", "he", "liang", \
    "xian", "fu", "liang", "can", "jing", "li", "yue", "lu", "ju", "qi", \
    "cui|sui", "bai", "zhang", "lin", "zong", "jing|qing", "guo|hua", "hua", "shen|san", "shen|san", \
    "tang", "bian", "rou", "mian", "hou", "xu", "zong", "hu", "jian", "zan", \
    "ci", "li", "xie", "fu", "nuo", "bei", "gu", "xiu", "gao", "tang", \
    "qiu", "jia", "cao", "zhuang", "tang", "mi|mei", "shen|san", "fen", "zao", "kang", \
    "jiang", "mo", "san", "san", "nuo", "xi|chi", "liang", "jiang", "kuai", "bo", \
    "huan", "shu", "ji", "han|xian", "nuo", "tuan", "nie", "li", "zuo", "di", \
    "nie", "diao|tiao", "lan", "si|mi", "si", "jiu", "xi|ji", "gong", "zheng", "jiao|jiu", \
    "you", "ji", "cha", "zhou", "xun", "yue|yao", "hong|gong", "ou|yu", "he|ge", "wan", \
    "ren", "wen", "wen", "qiu", "na", "zi", "tou", "niu", "fou", "jie|ji", \
    "shu", "chun|zhun", "pi|bi", "zhen", "miao|sha", "hong", "zhi", "ji", "fen", "yun", \
    "ren", "dan", "jin", "su", "fang|bang", "suo", "cui|zu", "jiu", "zha|za", "ba|ha", \
    "jin", "fu", "zhi", "qi", "zi", "chou|zhou", "hong", "zha|za", "lei|lv", "xi", \
    "fu", "xie|yi", "shen", "bo|bi", "shu|zhu", "qu", "ling", "zhu", "chao|shao", "gan", \
    "yang", "fu|fei", "tuo", "jin|tian|zhen", "dai", "chu", "shi", "zhong", "xuan|xian", "zu|qu", \
    "jiong", "ban", "qu", "mo", "shu", "zui", "kuang", "jing", "ren", "hang", \
    "xie|yi", "jie|ji", "zhu", "chou", "gua|kua", "bai|mo", "jue", "kuang", "hu", "ci", \
    "huan|geng", "geng", "tao", "jie|xie", "ku", "jiao|xiao", "quan", "ai|gai", "luo|lao", "xuan|xun", \
    "bing|beng", "xian", "fu", "gei|ji", "tong|dong", "rong", "diao|tiao|dao", "yin", "lei", "xie", \
    "juan", "chu|na|nv|xu", "hai|gai", "die", "tong", "si", "jiang", "xiang", "gui|hui", "jue", \
    "zhi", "jian", "juan|xuan", "chi|zhi", "wan|wen|man", "zhen", "lv", "cheng", "qiu", "shu", \
    "bang", "tong", "shao|xiao", "huan|wan", "qin|xian", "geng|bing", "xiu|xu", "ti", "xiu|tou", "xie", \
    "hong", "xi", "fu", "ting", "rui|shuai|sui", "dui", "kun", "fu", "jing", "hu", \
    "zhi", "xian|yan", "jiong", "feng", "ji", "xu", "ren", "zong|zeng", "lin|chen|shen", "duo", \
    "li|lie", "lv|lu", "liang", "chou|tao", "quan", "shao|chao", "qi", "qi", "zhun", "qi", \
    "wan", "qing|qian", "xian", "shou", "wei|yi", "qi|qing", "tao", "wan", "gang", "wang", \
    "beng", "zhui|chuo", "cai", "guo", "zu|cui", "lun|guan", "liu", "qi|yi", "zhan", "bi", \
    "chuo|chao", "ling", "mian", "qi", "qie", "tan|tian", "zong", "hun|gun", "zou", "xi", \
    "zi", "xing", "liang", "jin", "fei", "rui", "hun|mian|min", "yu", "cong|zong", "fan", \
    "lv|lu", "xu", "ying", "shang", "qi", "xu", "xiang", "jian", "ke", "xian", \
    "ruan", "mian", "ji|qi", "duan", "zhong|chong", "di", "min|mian", "miao|mao", "yuan", "xie|ye", \
    "bao", "si", "qiu", "bian", "huan", "geng", "cong|zong", "mian", "wei", "fu", \
    "wei", "xu|shu|tou", "gou", "miao", "xie", "lian", "zong", "bian|pian", "yun|gun", "yin", \
    "ti", "gua|wo", "zhi", "wen|yun", "cheng", "chan", "dai", "xia", "yuan", "zong", \
    "xu", "sheng|ying", "wei", "geng", "seon", "ying", "jin", "yi", "zhui", "ni", \
    "bang", "hu|gu", "hu|pan", "chao|cu|zhou", "jian", "cuo|ci", "quan", "shuang", "yun|wen", "xia", \
    "shuai|sui|cui", "xi|ji", "rong", "tao", "fu", "yun", "zhen|chen", "gao", "rong|ru", "hu", \
    "zai|zeng", "teng", "xian|xuan", "su", "zhen", "cong|zong", "tao", "huang", "cai", "bi", \
    "feng", "cu", "li", "suo|su", "yan|yin", "xi", "cong|zong", "lei", "zhuan|juan", "qian", \
    "man", "zhi", "lv", "mo|mu", "piao", "lian", "mi", "xuan", "cong|zong", "ji", \
    "shan|xian|sao", "cui|sui", "fan|po", "lv", "beng", "yi", "sao|zao", "miu|miao|liao|mou|mu", "yao|you|zhou", "qiang", \
    "sheng|hun", "xian", "xi|ji", "sha", "xiu", "ran", "xuan", "sui", "qiao|jue", "ceng|zeng", \
    "zuo", "zhi", "shan", "san", "lin", "ju|yu|jue", "fan", "liao|rao", "chuo|chao", "zun", \
    "jian", "rao", "chan", "rui", "xiu", "hui", "hua", "zuan", "xi", "qiang", \
    "yun", "da", "min|sheng|ying", "hui|gui", "xi|ji", "se", "jian", "jiang", "huan", "zao|sao|qiao", \
    "cong", "xie|jie", "he|jiao|zhuo", "bi", "tan|chan|dan", "yi", "nong", "sui", "yi|shi", "sha|shai", \
    "ru|xu", "ji", "bin", "qian", "lan", "fu|pu", "xun", "zuan", "qi", "peng", \
    "li|yao", "mo", "lei", "xie", "zuan", "kuang", "you", "xu", "lei", "xian|jian", \
    "chan", "jiao", "lu", "chan", "ying", "shan|cai", "rang|xiang", "xian|jian", "zui", "zuan", \
    "luo", "li|xi", "dao|du", "lan", "lei", "lian", "si", "jiao|jiu", "ou|yu", "hong|gong", \
    "zhou", "xian|qian", "he|ge", "yue|yao", "ji", "wan", "kuang", "ji", "ren", "wei", \
    "yun", "hong", "chun|quan|tun|zhun", "pi|bi", "miao|sha", "gang", "na", "ren", "cong|zong", "lun|guan", \
    "fen", "zhi", "wen", "bang|fang", "zhu", "zhen", "niu", "shu", "xian", "gan", \
    "xie|yi", "fu", "lian", "qu|zu", "shen", "xi", "zhi", "zhong", "chao|cu|zhou", "ban", \
    "fei|fu", "chu", "chao|shao", "shi|yi", "jing", "dai", "bang", "rong", "ji|jie", "ku", \
    "rao", "die", "hang", "gui|hui", "gei|ji", "xuan|xun", "jiang", "luo|lao", "jue", "jiao|xiao", \
    "tong", "bing|geng", "shao|xiao", "juan|xuan", "tou|xiu", "xi", "rui|shuai|sui", "tao", "ji", "ti", \
    "ji", "xu", "ling", "ying", "xu", "qi|yi", "fei", "chuo|chao", "shang", "gun|hun", \
    "min|sheng|ying", "wei|yi", "mian", "shou", "beng", "chou|diao|tao", "tao", "liu", "quan", "zong|zeng", \
    "zhan", "wan", "lv|lu", "chuo|zhui", "zi", "ke", "xiang", "jian", "mian", "lan", \
    "ti", "miao", "ji|qi", "wen|yun", "hui", "si", "duo", "duan", "bian|pian", "xian", \
    "gou", "zhui", "huan", "di", "lv", "bian", "min|mian", "yuan", "jin", "fu", \
    "rong|ru", "chen|zhen", "feng", "shuai|sui|cui", "gao", "chan", "li", "yi", "jian", "bin", \
    "piao", "man", "lei", "ying", "suo|su", "miu|miao|mou", "sao|zao", "xie", "liao|rao", "shan", \
    "ceng|zeng", "jiang", "qian", "zao|sao|qiao", "huan", "he|jiao|zhuo", "zuan", "fou", "xie", "gang", \
    "fou", "kui|que", "fou", "qi", "bo", "ping", "xiang", "zhao|diao", "gang", "ying", \
    "ying", "qing", "xia", "guan", "zun", "tan", "cang", "qi", "weng", "ying", \
    "lei", "tan", "lu", "guan", "wang", "si|wang", "gang|wang", "wang", "han", "ra", \
    "luo", "fu", "mi|shen", "fa", "gu", "zhu", "ju", "mao|meng", "gu", "min", \
    "gang", "ba", "gua", "ti|kun", "juan", "fu", "shen", "yan", "zhao", "zui", \
    "gua|guai", "zhuo", "yu", "zhi", "an", "fa", "lan", "shu", "si", "pi", \
    "ma", "liu", "ba|pi", "fa", "li", "chao", "wei", "bi", "ji", "zeng", \
    "chong", "liu", "ji", "juan", "mi", "zhao", "luo", "pi", "ji", "ji", \
    "luan", "yang", "mi", "qiang", "da", "mei", "yang|xiang", "you", "you", "fen", \
    "ba", "gao", "yang", "gu", "qiang|you", "yang|zang", "mei|gao", "ling", "yi|xi", "zhu", \
    "di", "xiu", "qian|qiang", "yi", "xian|yan|yi", "rong", "qun", "qun", "qian|qiang", "huan", \
    "suo|zui", "xian|yan", "xi|yi", "yang", "kong|qiang", "xian|qian", "yu", "geng|lang", "jie", "tang", \
    "yuan", "xi", "fan", "shan", "fen", "shan", "lian", "lei|lian", "geng|lang", "nou", \
    "qiang", "chan", "hu|yu", "gong|hong", "yi", "chong", "weng", "fen", "hong", "chi", \
    "chi", "cui", "fu", "xia", "ben|pen", "yi", "la", "yi", "po|pi|bi", "ling", \
    "lu|liu", "zhi", "yu|qu", "xi", "xie", "xiang", "xi", "xi", "ke", "qiao", \
    "hui", "hui", "shu|xiao", "sha", "hong", "jiang", "zhai|di", "cui", "fei", "zhou|dao", \
    "sha", "chi", "zhu", "jian", "xuan", "chi", "pian", "zong", "wan", "hui", \
    "hou", "he|li", "hao|he", "han", "ao", "piao", "yi", "lian", "qu|hou", "ao", \
    "lin", "pen", "qiao", "ao", "fan", "yi", "hui", "xuan", "dao", "yao", \
    "lao", "uu", "kao", "mao", "zhe", "shi|qi", "gou", "gou", "gou", "die", \
    "die", "er|neng", "shua", "ruan|nuo", "er|nai", "nai|neng", "zhuan|duan", "lei", "ting", "zi", \
    "geng", "chao", "hao|mao", "yun", "pa|ba", "pi", "yi|chi", "si", "qu|chu", "jia", \
    "ju", "huo", "chu", "lao", "lun", "jie|ji", "tang", "ou", "lou", "nou", \
    "jiang", "pang", "ze|zha", "lou", "ji", "lao", "huo", "you", "mo", "huai", \
    "er|reng", "yi", "ding", "xie|ye", "da|zhe", "song", "qin", "ying|yun", "chi", "dan", \
    "dan", "hong", "geng", "zhi", "uu", "nie|she|ye|zhe", "dan", "zhen", "che", "ling", \
    "zheng", "you", "wa|tui", "liao|liu", "long", "zhi", "ning", "tiao", "nv|er", "ya", \
    "zhe|tie", "guo", "sei", "lian", "hao", "sheng", "lie", "pin", "jing", "ju", \
    "bi", "di|zhi", "guo", "wen", "xu", "ping", "cong", "ding", "uu", "ting", \
    "ju", "cong", "kui", "lian", "kui", "cong", "lian", "weng", "kui", "lian", \
    "lian", "cong", "ao|you", "sheng", "song", "ting", "kui", "nie|she|ye|zhe", "te|zhi", "dan", \
    "ning", "qie", "jian|ni", "ting", "ting", "long", "yu", "nie|pianpang", "zhao", "si", \
    "su", "si|yi", "su", "si|ti", "zhao", "zhao", "rou|ru", "yi", "lei|le", "ji", \
    "qiu", "ken", "cao", "ge|qi", "di|bo", "huan", "huang", "chi", "ren", "xiao", \
    "ru", "zhou", "yuan", "du", "gang", "rong|chen", "gan", "cha", "wo", "chang", \
    "gu", "shi|zhi", "han|qin", "fu|lu", "fei", "fen|ban", "pei", "feng|pang", "jian|xian", "fang", \
    "chun|tun|zhun|zhuo", "you", "na|nv", "ang|gang|hang", "ken", "ran", "gong", "yu|yo", "wen", "yao", \
    "qi", "bi|pi", "qian|xu", "xi|bi", "xi", "fei|pie", "ken", "jing", "tai", "shen", \
    "zhong", "chan|zhang", "xi|xian|xie", "chen|shen", "wei", "zhou", "die", "da|dan|tan", "fei|bi", "ba", \
    "bo", "chun|qu|xu", "tian", "bei", "gu|gua|hu", "tai", "fei|zi", "fei|ku", "shi|zhi", "ni", \
    "ping|peng", "ci|zi", "fu|zhou", "pang|pan", "zhen|zhun", "xian", "zuo", "pei", "jia", "qing|sheng|xing", \
    "chi|di|zhi", "bao|pao", "mu", "qu", "hu", "ke", "chi", "yin", "xu", "yang", \
    "long", "dong", "ka", "lu", "jing|keng", "nu", "yan", "pang", "kua", "yi", \
    "guang", "gai|hai", "ge|ga", "dong", "zhi|chi", "jiao|xiao", "xiong", "xiong", "er", "an|e", \
    "heng", "pian", "neng|nai", "zi", "gui|kui", "cheng|zheng", "tiao", "zhi", "cui", "mei", \
    "xi|xian|xie", "cui", "xie|xian|xi", "mai|mo", "mai|mo", "ji", "xie|xian|xi", "nin", "kuai", "sa", \
    "zang", "qi", "nao", "mi", "nong", "ji|luan", "wan|wen", "bo", "wen", "huan|wan", \
    "xiu", "jiao|jue", "jing|keng", "rou|you", "heng", "cuo|qie", "luan|lie", "shan|chan", "ting", "mei", \
    "chun", "shen", "jia|qian", "te|de", "zui|juan", "ji|cu", "xiu|you|xiao", "xin|chi", "tui|tuo", "pao", \
    "cheng", "nei|tui", "pu|fu", "dou", "tui|tuo", "niao", "nao", "pi", "gu", "luo", \
    "lei|li", "lian", "zhang|chang", "cui|sui", "jie", "liang|lang", "shui", "bi|pai|pi", "biao", "lun", \
    "pian", "guo|lei", "juan|quan|kui", "chui|hou", "dan", "tian", "nei", "jing", "nai", "la|xi", \
    "ye", "yan|a", "ren|dian", "shen", "zhui|chuo", "fu", "fu", "ju", "fei", "kong|qiang", \
    "wan", "dong", "bi|pai|pi", "guo|huo", "zong", "ding", "wo", "mei", "ruan|nen|ni", "zhuan|dun", \
    "chi", "cou", "luo", "ou", "di", "an", "xing", "nao", "shu|yu", "shuan", \
    "nan", "yun", "zhong", "rou", "e", "sai", "dun|tu", "yao", "jian|qian", "wei", \
    "jiao|jue", "yu", "jia", "duan", "bi", "chang", "fu", "xian", "ni", "mian", \
    "wa", "teng", "tui", "bang|pang", "xian|qian", "lv", "wa", "shou", "tang", "su", \
    "zhui", "ge", "yi", "bo|lie|po", "liao", "ji", "pi", "xie", "gao", "lv", \
    "bin", "ou", "chang", "lu|biao", "guo|huo", "pang", "chuai", "biao|piao", "jiang", "lu|fu", \
    "tang", "mo", "xi", "zhuan|chuan", "lu", "hao|jiao", "ying", "lv|lou", "zhi", "xue", \
    "cen", "lian|lin", "tong|chuang", "peng", "ni", "chuai|zha|zhai", "liao", "cui", "gui|kui", "xiao", \
    "teng|tun", "pan|fan", "zhi", "jiao", "shan", "wu|hu", "cui", "run|yin", "xiang", "sui|wei", \
    "fen", "ying", "shan|dan", "zhua", "dan", "kuai", "nong", "tun", "lian", "bi|bei", \
    "yong", "ju|jue", "chu", "yi", "juan", "ge|la", "lian", "sao", "tun", "gu", \
    "qi", "cui", "bin", "xun", "ru|nao", "wo|yue", "zang", "xian", "biao", "xing", \
    "kun", "la|lie", "yan", "lu", "huo", "za", "luo", "qu", "zang", "luan", \
    "luan|ni", "za|zan", "chen", "xian|qian", "wo", "guang|jiong", "zang|cang", "lin", "jiong|guang", "zi", \
    "jiao", "nie", "chou|xiu", "ji", "gao|gu|hao", "chou|xiu", "bian|mian", "nie", "die|zhi", "zhi|zhui", \
    "ge", "jian", "zhi|die", "zhi|jin", "xiu", "tai", "zhen", "jiu", "xian", "kui|yong|yu", \
    "cha", "yao", "yu", "chong|chuang|zhong", "que|xi", "que|tuo|xi", "jiu", "yu", "yu", "xin|xing", \
    "ju", "jiu", "xin|wen", "gua|she", "she|shi", "she", "jiu", "shi", "ran|tan", "shu|yu", \
    "shi", "tan|tian", "tan", "pu", "hu|pu", "guan", "hua|qi", "tian", "chuan", "shun", \
    "xia", "wu", "zhou", "dao", "chuan|xiang", "shan", "yi", "fan", "pa", "tai", \
    "fan", "ban", "chuan|fan", "hang", "fang", "ban|pan|bo", "bi", "lu", "zhong", "jian", \
    "cang", "ling", "zhou|zhu", "ze", "duo|tuo", "bo", "xian", "ge", "chuan", "xia", \
    "lu", "qiong|hong", "feng|pang", "xi", "kua", "fu", "zao", "feng", "li", "shao", \
    "yu", "lang", "ting", "uu", "wei", "bo", "meng", "nian|qian", "ju|keo", "huang", \
    "shou", "zong|ke", "bian", "mu|mo", "die", "dou", "bang", "cha", "yi", "sou", \
    "cang", "cao", "lou", "dai", "xue", "yao|tiao", "chong|tong|zhuang", "deng", "dang", "qiang", \
    "lu", "yi", "ji", "jian", "huo|wo", "meng", "qi", "lu", "lu", "chan", \
    "shuang", "gen|hen", "liang", "jian", "jian", "se|shai", "yan", "bo|fu|pei", "ping", "yan", \
    "yan", "cao", "cao|ao", "yi", "le|ji", "ding|ting", "jiao|qiu", "ai|yi", "nai|reng", "tiao", \
    "jiao", "jie", "peng", "wan", "yi", "cha", "mian", "mi|mie", "gan", "qian", \
    "xu|yu", "xu|yu", "di|que|shao|xiao", "xiong", "du", "xia|hu", "qi", "huang|mang|wang", "zi", "hui|hu", \
    "sui", "zhi", "xiang", "pi|bi", "fu", "tun|chun", "wei", "wu", "zhi", "qi", \
    "shan|wei", "wen", "qian", "ren", "fu|fou", "kou", "jie|gai", "hu|lu", "xu|zhu", "ji", \
    "qin|yin", "chi|qi", "yuan|yan", "fen", "ba|pa", "rui|ruo", "xin", "ji", "hua", "hua", \
    "fang", "wu|hu", "jue", "ji", "zhi", "yun", "qin", "ao", "zou|chu", "mao", \
    "ya", "fu|fei", "reng", "hang", "cong", "yin|chan", "you", "bian", "yi", "qie", \
    "wei", "li", "pi", "e", "wan|xian", "chang", "cang", "zhu", "su", "di|ti", \
    "yu|yuan|yun", "ran", "lian|ling", "tai", "tiao|shao", "di", "miao", "qing", "li|ji", "yong", \
    "he|ke", "mu", "bei", "bao|biao|pao", "gou", "min", "yi", "yi", "ju|qu", "pi|pie", \
    "ruo|re", "gu|hu|ku", "zhu|ning", "ni", "pa|bo", "bing", "chan|shan|tian", "xiu", "yao", "xian", \
    "ben", "hong", "yang|ying", "zha|zuo", "dong", "ju|cha", "die", "nie", "gan", "hu", \
    "peng|ping", "mei", "fu|pu", "sheng|rui", "gua|gu", "bi|bie", "wei", "fu|bo", "zhu|zhuo", "mao", \
    "fan", "qie|jia", "mao", "mao", "ba|fei|pei", "ci|zi", "mo", "zi", "zhi", "chi", \
    "ji", "jing", "long", "cong", "niao", "uu", "xue", "ying", "qiong", "ge|luo", \
    "ming", "li", "rong", "yin", "gen|jian", "qian|xi", "chai|zhi", "chen", "wei|yu", "xiu|hao", \
    "zi", "lie", "wu", "duo|ji", "gui", "ci", "chong|jian", "ci", "hou|gou", "guang", \
    "huang|mang", "cha|chi", "jiao|qiao|xiao", "niao|jiao", "fu", "yu", "zhu", "zi|ci", "jiang", "hui", \
    "yin", "cha", "fa|pei", "rong", "ru", "chong", "mang|mu", "tong", "zhong", "qian", \
    "zhu", "xun", "huan", "fu", "chuo|quan", "gai", "ta|da", "jing", "xing", "chuan", \
    "cao|zao", "jing", "er", "an", "qiao", "chi|qi", "ren", "jian", "yi|ti", "huang|kang", \
    "peng|ping", "li", "jin|qian", "lao|cha", "shu", "zhuang", "da", "jia", "rao|yao", "bi", \
    "ce", "qiao", "hui", "ji|qi", "dang|tang", "yu", "rong", "hun|xun", "ying|xing", "luo", \
    "ying", "qian|xun", "jin", "sun", "yin", "mai", "hong", "zhou", "lue|shuo|yao|yue", "du", \
    "wei", "li", "dou", "fu", "ren", "yin", "he", "bi", "bu|pu", "yun", \
    "di", "cha|shu|tu|ye", "sui|wei", "sui", "cheng", "chen|nong", "wu", "bie", "xi", "geng", \
    "li", "fu|pu", "zhu", "mo", "chi|li", "zhuang", "ji|zuo", "tuo", "qiu", "sha|suo", \
    "suo", "chen", "feng|peng", "ju", "mei", "meng|qing|xi", "xing", "jing|ying", "che", "shen|xin", \
    "jun", "yan", "ting", "di|diao|you", "cuo", "guan|wan", "han", "xiu|you", "cuo", "jia", \
    "wang", "you|su", "niu|rou", "xiao|shao", "wan|xian", "liang|lang", "piao|fu", "e", "mo|wu", "mian|wan|wen", \
    "jie", "nan", "mu", "kan", "lai", "lian", "shi", "wo", "tu", "xian|lian", \
    "huo", "you", "ying", "ying", "neus", "chun", "mang", "mang", "ci", "wan|yun|yu", \
    "jing", "di", "qu", "dong", "guan|jian", "zou|cuan|chu", "gu", "la", "lu", "ju", \
    "wei", "jun", "ren|nie", "kun", "ge|he", "pu", "zi|zai", "gao", "guo", "fu", \
    "lun", "chang", "chou", "song", "chui", "zhan", "men", "cai", "ba", "li", \
    "tu", "bo", "han", "bao", "qin", "juan", "si|xi", "qin", "di", "sha|jie", \
    "bei|bo|pu", "dang", "jin", "zhao|qiao", "zhi|tai|chi", "geng", "hua|kua", "gu", "ling", "fei", \
    "jin|qin", "an|yan", "wang", "beng", "zhou", "yan|yu", "zu|ju", "jian", "lin", "tan", \
    "jiao|shu", "tian", "dao", "hu", "ji|qi", "he", "cui", "tao", "chun", "ba|bei|bi|pi", \
    "chang", "huan", "fei|fu", "lai", "qi", "meng|ming", "ping", "wei", "dan|wei", "sha", \
    "huan|zhui", "yan|juan", "yi", "shao|tiao", "qi|ji|ci", "guan|wan", "ce", "nai", "zhen", "tuo|ze", \
    "jiu", "tie", "luo", "bi", "yi", "meng", "be", "pao", "ding", "ying", \
    "ying", "ying", "xiao", "sa", "jiao|qiu", "ke", "xiang", "wan", "yu|ju", "yu", \
    "fu|bei", "lian", "xuan", "xuan", "nan", "ce", "wo", "chun", "xiao|shao", "yu", \
    "bian|pian", "mao|mu", "an", "e", "luo|la|lao", "ying", "huo|kuo", "kuo", "jiang", "mian|wan", \
    "ze|zuo", "zuo", "zu|ju", "bao", "rou", "xi", "she|ye", "an|yan", "qu", "jian", \
    "fu", "lv", "jing|jian", "pen|fen", "feng", "hong", "hong", "hou", "yan|xing", "tu", \
    "zhu|zhuo|zhe", "zi", "xiang", "shen|ren", "ge", "qia", "jing|qing", "mi", "huang", "shan|shen", \
    "bei|pu", "gai", "dong|zhong", "zhou", "qian|jian", "wei", "bo", "wei", "pa", "ji", \
    "hu", "zang", "jia|xia", "duan", "yao", "jun|sui", "chuang|cong", "quan", "wei", "zhen|qian", \
    "kui", "ding|ting", "hun|xun", "xi", "shi", "qi", "lan", "zong", "yao", "yuan", \
    "mei", "yun", "shu", "di", "zhuan", "guan", "ran", "xue", "chan", "kai", \
    "kui|kuai", "uu", "jiang", "ju|liu|lou|lv", "wei|hua", "pai", "yong|you", "hui|sou", "yin", "shi", \
    "chun", "shi", "yun", "zhen", "lang", "ru|na", "meng|weng", "li", "que", "suan", \
    "yuan|huan", "li", "ju", "xi", "bang|pang", "chu", "xu|shu", "tu", "liu", "huo|wo", \
    "dian", "qian", "zu|ju", "po", "cuo", "yuan", "chu", "yu", "kuai", "pan", \
    "pu", "bo|pu", "na", "shuo", "xi", "fen", "yun", "zheng", "jian", "ji", \
    "ruo", "cang", "en", "mi", "gao|hao", "sun", "qin|zhen", "ming|mi", "sou", "xu", \
    "liu", "xi", "gu", "lang", "rong", "weng", "gai|ge", "cuo", "shi", "tang", \
    "luo", "ru", "sui|suo", "xuan", "bei", "yao|zhuo", "gui", "bi", "zong", "gun", \
    "zuo", "tiao", "ce", "pei", "la|lan", "uu", "ji", "li", "shen", "lang", \
    "yu", "ling", "ying", "ma|mo", "tiao|di|you|diao", "xiu|tiao", "mao", "tong", "zhu|chu", "peng", \
    "an", "lian", "cong|zong", "xi", "ping", "xu|ou|qiu|fu", "jin", "tuan|chun", "jie", "wei", \
    "tui", "cao", "yu", "yi", "zi|ju", "liao|lu", "bi", "lu", "xu", "bu", \
    "zhang", "lei", "qiang|jiang", "man", "yan", "ling", "ji|xi", "piao|biao", "gun", "han", \
    "di", "su", "lu|cu", "she", "shang", "di", "mie", "hun|xun", "man|wan", "bo", \
    "dai|di", "cuo|cu", "zhe", "san|shen", "xuan", "wei|yu", "hu", "ao", "mi", "lou|lv", \
    "chuo|cou|cu", "zhong", "ca|cai|sa", "po|bo", "jiang", "mi", "cong|chuang", "niao", "hui", "juan|jun", \
    "yin", "jian|shan", "nian|yan", "shu", "yin", "guo", "chen", "hu", "sha", "kou", \
    "qian", "ma", "zang|cang", "ze", "qiang|se", "dou", "lian|xian", "lin", "kou", "ai", \
    "bi|bie|pie", "li", "wei", "ji", "xun|qian", "sheng", "fan|bo", "meng", "ou", "chan", \
    "dian", "tan|xun", "jiao|qiao", "juan|rui", "juan|rui", "lei", "yu", "jiao|qiao", "zhu|chu", "hua|kua", \
    "jian", "mai", "yun", "bao", "you", "qu", "lu", "rao|yao", "hui", "e", \
    "ti", "fei", "jue", "jue|zhuo|zui", "fa|fei", "ru", "fen|fei", "kui|kuai", "shun", "rui", \
    "ya", "xu", "fu", "jue", "dang|tang", "wu", "dong", "si", "xiao", "xi", \
    "long|sa", "wen|yun", "shao", "ji|qi|qin", "jian", "wen|yun", "sun", "ling", "yu", "xia", \
    "weng|yong", "ji|qie", "hong", "si", "nong", "lei", "xuan", "yun|wen", "yu", "xi|xiao", \
    "hao", "bao|bo", "hao", "ai", "wei", "hui", "hui", "ji", "zi|ci", "xiang", \
    "luan|wan", "mie", "yi", "leng", "jiang", "can", "shen", "qiang|se", "lian", "ke", \
    "yuan", "da", "zhi|ti", "tang", "xue", "bai|bi|bo", "zhan", "sun", "lian|xian", "fan", \
    "ding", "xie", "gu", "xie", "shu|zhu", "jian", "hao|kao", "hong", "sa", "xin", \
    "xun", "yao", "bai", "cou|shu|sou", "shu", "xun", "dui", "pin", "yuan|wei", "ning", \
    "zhou|chou", "mai|wo", "ru", "piao", "tai", "qi|ji|ci", "zao", "chen", "zhen", "er", \
    "ni", "ying", "gao", "cong", "hao|xiao", "qi", "fa", "jian", "yu|xu", "kui", \
    "ji|jie", "bian", "di|diao", "mi", "lan", "jin", "cang|zang", "miao|mo", "qiong", "qie", \
    "xian", "uu", "ou", "xian|qian", "su", "lv", "yi", "xu", "xie", "li", \
    "yi", "la", "lei", "jiao", "di", "zhi", "bei", "teng", "yao|yue|shuo", "mo", \
    "huan", "biao|pao", "fan", "cou|shu|sou", "tan", "tui", "qiong", "qiao", "wei", "liu", \
    "hui", "ou", "gao|kao", "yun|wen", "bao", "li", "zhu|shu", "zhu|chu", "ai", "lin", \
    "zao", "xuan", "qin", "lai", "he|huo", "ze|tuo", "e|wu", "rui", "rui", "ji|qi", \
    "heng", "lu", "su", "tui", "mang", "yun|wen", "ping|pin", "yu", "xun", "ji", \
    "jiong", "xuan", "mo", "qiu", "su", "jiong", "feng", "bo|nie", "bi|bo", "xiang|rang", \
    "yi", "xian", "yu", "ju", "lian|xian", "lian|xian", "yin", "qiang", "ying", "long", \
    "tou", "hua", "yue", "ling", "ju|qu", "yao", "fan", "mei", "lan|han", "kui|hui", \
    "lan", "ji", "dang|tang", "man", "lei", "lei", "hui|hua", "feng|song", "zhi", "wei", \
    "kui", "zhan", "huai", "li", "ji", "mi", "lei", "huai", "luo", "ji", \
    "kui", "lu", "jian", "sal", "teng", "lei", "quan", "xiao", "yi", "luan", \
    "men", "bie", "hu|pianpang", "hu", "lu", "nue|nve", "bi|lv", "si|ti|zhi", "xiao", "qian", \
    "chu|ju", "hu", "xu", "cuo", "fu", "xu", "xu", "lu", "hu", "yu", \
    "hao", "jiao|hao", "ju", "guo", "bao", "yan", "zhan", "zhan", "kui", "bin", \
    "se|xi", "shu", "chong|hui", "qiu", "dao|diao", "ji", "qiu", "ding|cheng", "shi", "uu", \
    "jue", "zhe", "she|ye", "yu", "gan|han", "zi", "gong|hong|jiang", "hui", "meng", "ge", \
    "sui", "xia|ha", "chai", "shi", "yi", "ma", "xiang", "fang|bang", "e", "ba", \
    "chi", "qian", "wen", "wen", "rui", "beng|bang", "pi", "yue", "yue", "jun", \
    "qi", "tong", "yin", "zhi|qi", "can|tian", "yuan|wan", "que|jue", "you|hui", "qin|qian", "qi", \
    "zhong", "ya", "ci|hao", "mu", "wang", "fen", "fen", "hang", "gong|zhong", "zao|zhao", \
    "fu", "ran", "jie", "fu", "chi", "dou", "bao|pao", "xian", "ni", "dai|te", \
    "qiu", "you|zhu", "zha", "ping", "chi|di", "niu|you", "he|ke", "han", "ju", "li", \
    "fu", "ran|tian", "zha", "qu|gou", "pi", "pi|bo", "xian", "zhu", "diao", "bie", \
    "bing", "gu", "zhan", "ju|qu", "she|yi", "tie", "ling", "gu", "dan", "gu", \
    "ying", "li", "cheng", "qu", "mao|mou", "ge|luo", "ci", "hui", "hui", "mang|bang", \
    "fu", "yang", "jue|wa", "lie", "zhu", "yi", "xian", "kuo|she", "jiao", "li", \
    "xu|yi", "ping", "jie|qie", "ge|ha", "she", "yi", "wang", "mo", "gong|qiong", "qie|ni", \
    "gui", "qiong", "zhi", "man", "lao|e", "zhe", "jia", "nao", "si", "qi", \
    "xing", "jie", "qiu", "xiao|shao", "yong", "jia", "tui|yue", "che", "bei", "e|yi", \
    "han", "shu", "xuan", "feng", "shen", "zhen|shen", "pu|fu", "xian", "zhe", "wu", \
    "fu", "li", "liang|lang", "bi", "chu|yu", "xuan|yuan", "you", "jie", "dan", "dan|yan", \
    "dian|ting", "dian", "tui|yue", "hui", "wo", "zhi", "song", "bei|fei|pei", "ju", "mi", \
    "qi", "qi", "yu", "jun", "la|zha", "meng", "qiang", "xi|si", "xi", "lun", \
    "li", "die", "diao|tiao", "tao", "kun", "han", "han", "guo|yu", "bang", "fei", \
    "miao|pi", "wei", "dun|tun", "xi|yi", "yuan|yun", "suo", "juan|quan", "qian", "rui|wei", "ni", \
    "jing|qing", "tong|wei", "liang", "guo|luo", "wan", "dong", "e", "ban", "di|zhuo", "wang", \
    "can", "mi", "ying", "guo", "chan", "uu", "la", "ke", "ji|jie", "he|xie", \
    "ting", "mao", "xu|xie", "mian", "yu", "jie", "li|long|shi", "xuan", "huang", "yan", \
    "bian|pian", "rou|nao", "wei", "fu", "yuan", "mei", "wei", "fu", "ruan", "xie", \
    "you", "qiu|you", "mao|wu", "xia|ha", "ying", "shi", "zhong|chong", "tang", "zhu", "zong", \
    "ti|chi", "fu", "yuan", "kui", "meng", "la", "du|dai", "hu", "qiu", "die|tie", \
    "li|xi", "wo|luo", "ao|yun", "yu|qu", "nan", "lou", "chun", "rong", "ying", "jiang", \
    "tui|ban", "lang", "bang|pang", "si", "ci|xi", "ci", "xi|qi", "yuan", "weng", "lian", \
    "sou", "ban|pan", "rong", "rong", "ji", "wu", "xiu", "han", "qin", "yi|si", \
    "bi|pi", "hua", "tang", "yi", "du", "neng|nai", "xia|he", "hu", "gui|hui", "ma", \
    "ming", "yi", "wen", "ying", "teng|te", "zhong", "cang", "so", "qi", "man", \
    "tiao", "shang", "shi|zhe", "cao", "chi", "dai|di", "ao", "lu", "wei", "zhi|die", \
    "tang", "chen", "piao", "ju|qu", "pi", "yu", "jian|chan", "luo", "lou", "qin", \
    "zhong", "yin", "jiang", "shuai", "wen", "xiao", "man|wan", "zhe", "zhe", "ma|mo", \
    "ma", "guo|yu", "liao|liu", "mao|meng", "xi", "cong", "li", "man", "xiao", "chang", \
    "zhang", "mang|meng", "xiang", "mo", "zui", "si", "qiu", "te", "zhi", "peng", \
    "peng", "qiao|jiao", "qu", "bie", "liao", "fan|pan", "gui", "xi", "ji|qi", "zhuan", \
    "huang", "ben|fei", "lao|liao", "jue", "jue", "hui", "yin|xun", "chan|ti", "jiao", "shan", \
    "rao|nao", "xiao", "wu|mou", "chong|zhong", "xun", "si", "chu", "cheng", "dang", "li", \
    "xie", "shan|dan", "ji|yi", "jing", "da", "chan", "qi|ji", "ci|ji", "xiang", "she", \
    "guo|luo", "kem|qin", "ying", "chai", "li", "zei", "xuan", "lian", "shu|zhu", "ze", \
    "xie", "mang", "xie", "qi", "rong", "jian", "meng", "hao", "ru", "huo|yue", \
    "zhuo", "jie", "bin", "he", "mie", "fan", "lei", "jie", "la", "mian|min", \
    "li|luo", "chun", "li", "qiu", "nie", "lu", "du", "xiao", "chu|zhu", "long", \
    "li", "long", "pang", "ye", "pi", "rang|shang|nang", "gu|ye", "juan", "ying", "shu", \
    "xi", "can", "qu", "huan|quan", "du", "can", "man", "jue|qu", "jie", "shu|zhu", \
    "zhuo", "xue|xie", "huang", "nv", "fou|pei", "nv", "xin", "zhong", "mai", "er", \
    "ka|kai", "mie", "xi", "xing|hang|heng", "yan", "kan", "yuan", "qu", "ling", "xuan", \
    "shai|shu", "xian", "tong|dong", "long|xiang", "jie", "xian|yu", "ya|yu", "hu", "wei", "dao", \
    "chong", "wei", "dao", "zhun", "heng", "qu", "yi", "pianpang|yi", "bu", "gan", \
    "yu", "biao", "cha", "yi", "shan", "chen", "fu", "gun", "fen|pen", "shuai|cui", \
    "jie", "na", "zhong", "dan", "yi", "zhong", "zhong", "jie", "zhi|ti", "xie", \
    "ran", "zhi", "ren", "qin", "jin|qin", "jun", "yuan", "mei|yi", "chai", "ao", \
    "niao", "yi|hui", "ran", "jia", "tuo", "ling", "dai", "bao|pao", "bao|pao", "yao", \
    "zuo", "bi", "shao", "tan|zhan", "ju|jie", "ke|he", "xue", "xiu", "zhen", "tuo|yi", \
    "pa", "bo|fu", "di", "mo|wa", "fu", "gun", "zhi", "zhi", "ran", "fan|pan", \
    "yi", "mao|mou", "tuo", "na|jue", "gou", "xuan", "zhe|chan", "qu", "bei|bi|pi", "yu", \
    "xi", "mi|ni", "bo", "uu", "fu", "chi|nuo", "chi|qi", "ku", "ren", "jiang", \
    "qia|jia|jie", "zun|jian", "mo|bo", "jie", "er", "ge|luo", "ru", "zhu", "gui", "yin", \
    "cai", "lie", "ka", "xing", "zhuang", "dang", "sed", "kun", "ken", "niao", \
    "shu", "jia|xie", "kun", "cheng", "li", "juan", "shen", "bao|pou", "jie|ge", "yi", \
    "yu", "zhen", "liu", "qiu", "qun", "ji", "yi", "bu", "zhuang", "shui", \
    "sha", "qun", "li", "lian", "chan|lian", "ku", "jian", "bao|pou|xiu", "chan|tan", "bi|pi", \
    "kun", "tao", "yuan", "ling", "chi", "chang", "chou|dao", "duo", "biao", "liang", \
    "shang|chang", "fei|pei", "pei|fei", "fei", "yuan|gun", "luo", "guo", "an|yan", "du", "xi|ti", \
    "zhi", "ju", "yi|qi", "qi|ji", "guo", "gua", "ken", "qi", "ti", "ti|shi", \
    "fu", "chong|zhong", "xie", "bian|pian", "die", "kun", "duan|tuan", "xiu|you", "xiu|you", "he", \
    "yuan", "bao|pou", "bao", "fu", "tou|yu", "tuan", "yan", "yi|hui", "bei", "chu|zhu", \
    "lv", "pao", "dan", "yun|wen", "ta", "gou", "da", "huai", "rong", "yuan", \
    "nu|ru", "nai", "jiong", "suo|cha", "ban|pan", "tui|tun", "chi", "sang", "niao", "ying", \
    "jie", "qian", "huai", "ku", "lian", "lan", "li", "die|xi|zhe", "shi", "lv", \
    "nie|yi", "die", "xie", "xian", "wei", "biao", "cao", "ji", "qiang", "sen|shan", \
    "bao|pou", "xiang", "bi", "pu|fu", "jian", "zhuan|juan", "jian", "cuo|cui", "ji", "chan|dan", \
    "za", "fan|bo", "bo|fei", "xiang", "xun|xin", "bie", "rao", "man", "lan", "ao", \
    "ze|yi", "hui|gui", "cao", "sui", "nong", "chan|dan", "lian|chan", "bi", "jin", "dang", \
    "shu|du", "tan|zhan", "bi", "lan", "pu|fu", "ru", "zhi", "tae", "shu|du", "wa", \
    "shi", "bai|bei", "xie", "bo", "chen", "lai", "long", "xi", "shan|xian", "lan", \
    "zhe", "dai", "ju", "zan|cuan", "shi", "jian", "pan", "yi", "lan", "ya", \
    "xi", "xi|ya", "yao", "ban|feng", "qin|tan", "fu", "fiao", "fu", "ba|po", "he", \
    "ji", "ji", "jian|xian", "guan", "bian", "yan", "gui|xu", "jue|jiao", "pian", "mao", \
    "mi", "mi", "pie|mie", "shi", "si", "chan|dan", "zhen", "jue|jiao", "mi", "tiao", \
    "lian", "yao", "zhi", "jun", "xi", "shan", "wei", "xi", "tian", "yu", \
    "lan", "e", "du", "qin|qing", "pang", "ji", "ming", "ying", "gou", "qu", \
    "zhan", "jin", "guan", "deng", "jian|bian", "luan|luo", "qu", "jian", "wei", "jue|jiao", \
    "qu", "luo", "lan", "shen", "ji|di", "guan", "jian|xian", "guan", "yan", "gui|xu", \
    "mi", "shi", "chan|dan|ji", "lan", "jue|jiao", "ji", "xi", "di|ji", "tian", "yu", \
    "gou", "jin", "qu", "jiao|jue", "qiu", "jin", "cu|chu", "gui|jue|kui", "zhi", "chao", \
    "ji", "gu", "dan", "zi|zui", "di|zhi", "shang", "xie|hua", "quan", "ge", "shi", \
    "jie|xie", "gui", "gong", "chu", "jie|xie", "hun", "qiu", "xing", "su", "ni", \
    "ji|qi", "lu", "zhi", "da|zha", "bi", "xing", "hu", "shang", "gong", "zhi", \
    "hu|xue", "chu", "xi|wei", "yi", "li|lu", "jue", "xi|wei", "yan", "xi|wei", "yan|yin", \
    "yan", "ding", "fu", "kao|qiu", "qiu", "jiao", "heng|hong|jun", "ji", "fan", "xun", \
    "diao", "hong", "cha|chai", "tao", "xu", "jie|ji", "yi|dan", "ren", "xun", "yin", \
    "shan", "qi", "tuo", "ji", "xun", "yin", "e", "fen|bin", "ya", "yao", \
    "song", "shen", "jin|yin", "xin|xi", "jue", "xiao|na", "ne", "chen", "you", "zhi", \
    "xiong", "fang", "xin", "chao|miao", "she", "yan", "sa", "zhun", "xu|hu", "yi", \
    "yi", "su", "chi", "he", "shen", "he", "xu", "zhen", "zhu", "zheng", \
    "gou", "zi", "zi", "zhan|dian|che", "gu", "fu", "jian", "die", "ling", "di|ti", \
    "yang", "li", "nu|nao|na", "pan", "zhou", "gan", "yi", "ju", "yao", "zha", \
    "tuo|duo", "dai|yi", "qu", "zhao", "ping", "bi", "xiong", "chu|qu", "ba|bo", "da", \
    "zu", "tao", "zhu", "ci", "zhe", "yong", "xu", "xun", "yi", "huang", \
    "he|ge", "shi", "cha|qie", "xiao", "shi", "hen", "cha|du", "gou|hou", "gui", "quan", \
    "hui", "jie", "hua", "gai", "xiang|yang", "wei", "shen", "zhou|chou", "dong|tong", "mi", \
    "dan|zhan", "ming", "luo|e|lue", "hui", "yan", "xiong", "gua", "er|chi", "bing", "tiao|diao", \
    "chi|yi", "lei", "zhu", "kuang", "kua|qu", "wu", "yu", "teng", "ji", "zhi", \
    "ren", "cu", "lang", "e", "kuang", "xi|yi", "shi", "ting", "dan", "bei", \
    "chan", "you", "keng", "qiao", "qin", "shua", "an", "yu", "xiao", "cheng", \
    "jie", "xian", "wu", "wu", "gao", "song", "bu", "hui", "jing", "shuo|shui|yue", \
    "zhen", "shuo|shui|yue", "du", "hua", "chang", "shui|shei", "jie", "ke", "jue|qu", "cong", \
    "xiao", "sui", "wang", "xian", "fei", "lai|chi", "ta", "yi", "ni|na", "yin", \
    "diao|tiao", "bei|pi", "zhuo", "chan", "chen", "zhun", "ji", "qi|ji", "tan", "zhui", \
    "wei", "ju", "qing", "dong", "zheng", "cuo|zuo|ze|zha", "zou|zhou", "qian", "zhuo", "liang", \
    "jian", "chu|ji", "hao|xia", "lun", "nie|shen", "biao", "hua", "bian|pian", "yu", "die|xie", \
    "xu", "pian", "shi|di", "xuan", "shi", "hun", "gua|hua", "e", "zhong", "di|ti", \
    "xie", "fu", "pu", "ting", "jian|lan", "qi", "tou|yu", "zi", "zhuan", "xi|shai", \
    "hui", "yin", "an|tou", "gan|xian", "nan", "chen", "feng", "chu|zhu", "yang", "yan", \
    "huang", "xuan", "ge", "nuo", "qi|xu", "mou", "ye", "wei", "xing", "teng", \
    "zhou|chou", "shan", "jian", "pao|po", "kui|dui", "huang", "huo", "ge", "hong|ying", "mi|mei", \
    "sou|xiao", "mi", "xi|xia", "qiang", "chen|zhen", "xue", "ti|si", "su", "bang", "chi", \
    "qian|zhan", "shi|xi|yi", "jiang", "quan|yuan", "xie", "xiao|he", "tao", "yao", "yao", "zhi", \
    "xu|yu", "biao|piao", "cong", "qing", "li", "mo", "mo", "shang", "ze|zhe", "miu", \
    "jian", "ze", "zu|jie", "lian", "lou|lv", "can|zao", "ou|xu", "gun", "xi|che", "shu|zhuo", \
    "ao", "ao", "jin", "zhe", "yi|chi", "xiao|hu", "jiang", "man", "chao|zhao", "han|xian", \
    "hua|wa", "chan|dan", "xu", "zeng", "se", "xi", "zha", "dui", "zheng", "xiao|nao", \
    "lan", "e", "ying", "jue", "ji", "zun", "jiao|qiao", "bo", "hui", "quan|zhuan", \
    "mo|wu", "jian|zen", "zha", "shi|zhi", "qiao", "tan", "zen", "pu", "sheng", "xuan", \
    "zao", "tan", "dang", "sui", "xian", "ji", "jiao", "jing", "lian|zhan", "nang|nou", \
    "yi", "ai", "zhan", "pi", "hui", "hui|hua", "yi", "yi", "shan", "rang", \
    "nou", "qian", "dui", "ta", "hu", "chou|zhou", "hao", "yi|ai", "ying", "jian|kan", \
    "yu", "jian", "hui", "du|dou", "ze|zhe", "juan|xuan", "zan", "lei", "shen", "wei", \
    "chan", "li", "yi|tui", "bian", "zhe", "yan", "e", "chou", "wei", "chou", \
    "yao", "chan", "rang", "yin", "lan", "chen|chan", "xie", "nie", "huan", "zan", \
    "yi", "dang", "zhan", "yan", "du", "pianpang|yan", "ji", "ding", "fu", "ren", \
    "ji", "ji|jie", "hong", "tao", "rang", "shan", "qi", "tuo", "xun", "yi", \
    "xun", "ji", "ren", "jiang", "hui", "ou|xu", "ju", "ya", "ne", "hu|xu", \
    "e", "lun", "xiong", "song", "feng", "she", "fang", "jue", "zheng", "gu", \
    "he", "ping", "zu", "shi|zhi", "xiong", "zha", "su", "zhen", "di|ti", "chao|chou|zhou", \
    "ci", "chu|qu", "zhao", "bi", "yi", "yi|dai", "kuang", "lei", "shi", "gua", \
    "shi", "jie|ji", "hui", "cheng", "zhu", "shen", "hua", "dan", "gou|hou", "quan", \
    "gui", "xun", "yi", "zheng", "gai", "xiang|yang", "cha|du", "hun", "xu", "chou|zhou", \
    "jie", "wu", "yu", "qiao", "wu", "gao", "you", "hui", "kuang", "shuo|shui", \
    "song", "ei|xi", "qing", "chu|zhu", "zhou|zou", "nuo", "du|dou", "zhuo", "fei", "ke", \
    "wei", "yu", "shui|shei", "mie|shen", "diao|tiao", "chan", "liang", "zhun", "sui", "tan", \
    "shen", "yi", "mou", "chen", "die|xie", "huang", "jian|lan", "xie", "nue|xue", "ye", \
    "wei", "e", "tou|yu", "xuan", "chan", "zi", "an|tou", "yan", "di|ti", "mi|mei", \
    "pian", "xu", "mo", "dang", "su", "xie", "yao", "bang", "shi|xi|yi", "qian|zhan", \
    "mi", "jin", "man", "ze|zhe", "jian", "miu", "tan", "jian|zen", "qiao", "lan", \
    "pu", "jue", "yan", "qian", "zhan", "chan|chen", "gu|yu", "qian", "hong", "xia", \
    "ji", "hong", "han", "hong|long", "xi|ji", "xi", "huo|hua", "liao", "han|gan", "du", \
    "long", "dou", "jiang", "kai|qi", "chi", "feng|li", "deng", "wan", "bi|bian", "shu", \
    "xian", "feng", "zhi", "zhi", "yan", "yan", "shi", "chu", "hui", "tun", \
    "yi", "dun|tun", "yi", "jian|yan", "ba", "hou", "e", "chu", "xiang", "huan", \
    "jian|yan", "ken|kun", "gai", "ju", "fu|pu", "xi", "huan|bin", "hao", "shu|xie|yu", "zhu", \
    "jia", "fen", "xi", "hu|bo", "wen", "huan", "ban|bin", "di", "zong", "fen", \
    "yi", "zhi", "bao", "chai", "an", "pi", "na", "pi", "gou", "duo|na", \
    "you", "diao", "mo", "si", "xiu", "huan", "ken|kun", "he|mo", "he|mo|hao", "ma|mo", \
    "an", "mao|mo", "li|mai", "ni", "bi", "yu", "jia", "tuan", "mao", "pi", \
    "xi", "yi", "lou|yu|ju", "mo", "chu", "tan", "huan", "jue", "bei", "zhen", \
    "yun|yuan", "fu", "cai", "gong", "dai|te", "yi", "hang", "wan", "pin", "huo", \
    "fan", "tan", "guan|wan", "ze|zhai", "zhi", "er", "zhu", "shi", "bi", "zi", \
    "er", "gui", "pian", "bian|fa", "mai", "dai|te", "sheng", "kuang", "fei|fu", "tie", \
    "yi", "chi", "mao", "he", "ben|bi", "lu", "lin", "hui", "gai", "pian", \
    "zi", "jia|gu", "xu", "zei", "jiao", "gai", "zang", "jian", "ying", "xun", \
    "zhen", "sha|she", "bin", "bin", "qiu", "sha|she", "chuan", "zang", "zhou", "lai", \
    "zan", "ci", "chen", "shang", "tian", "pei", "geng", "xian", "mai", "jian", \
    "sui", "fu", "tan|dan", "cong", "cong", "zhi", "ji", "zhang", "du", "jin", \
    "xiong|min", "chun", "yun", "bao", "zai", "lai", "feng", "cang", "ji", "sheng", \
    "yi|ai", "zhuan|zuan", "fu", "gou", "sai", "ze", "liao", "yi", "bai", "chen", \
    "wan", "zhi", "zhui", "biao", "bin|yun", "zeng", "dan", "zan", "yan", "pu", \
    "dan|shan", "wan", "ying", "jin", "gan|gong", "xian", "zang", "bi", "du", "shu", \
    "yan", "uu", "xuan", "long", "gan|gong", "zang", "bei", "zhen", "fu", "yun|yuan", \
    "gong", "cai", "ze|zhai", "xian", "bai", "zhang", "huo", "zhi", "fan", "tan", \
    "pin", "bian|fa", "gou", "zhu", "guan|wan", "er", "jian", "ben|bi", "shi", "tie", \
    "gui", "kuang", "dai|te", "mao", "bi|fei|fu", "he", "yi", "zei", "zhi", "jia|gu", \
    "hui", "zi", "lin", "lu", "zang", "zi", "gai", "jin", "qiu", "zhen", \
    "lai", "sha|she", "fu", "du", "ji", "shu", "shang", "ci", "bi", "zhou", \
    "geng", "pei", "tan|dan", "lai", "feng", "zhui", "fu", "zhuan|zuan", "sai", "ze", \
    "yan", "zan", "bin|yun", "zeng", "dan|shan", "ying", "gan|gong|zhuang", "chi", "xi", "ce|she", \
    "nan", "tong|xiong", "xi", "cheng", "he|shi", "cheng", "zhe", "xia", "tang", "zou", \
    "zou", "li", "jiu", "fu", "diao|zhao", "gan|qian", "qi", "shan", "qiong", "qin|yin", \
    "xian", "ci|zi", "jue|gui", "qin", "di|chi", "ci", "chen|nian|zhen", "chen|zhen", "die|tu", "qie|ju", \
    "chao|tiao", "di", "xi", "zhan", "jue|ju", "huo|yue", "cou|cu|qu", "jie|ji", "chi|qu", "chu", \
    "gua|huo", "xue|chi", "ci|zi", "tiao", "duo", "lie", "gan", "suo", "cu", "xi", \
    "zhao|diao", "su", "yin", "ju|qiu|qu", "jian", "que|ji|qi", "cheng|tang|zheng", "chuo|chao", "wei|cui", "lu", \
    "cou|cu|qu", "dang", "qiu|cu", "zi", "ti", "qu|cu", "chi", "huang|guang", "qiao|jiao|chao", "qiao", \
    "jiao", "zao", "yao|ti|yue", "er", "zan|zu", "zan|zu", "ju|zu", "pa", "bao|bo", "wu|ku|kua", \
    "ke", "dun", "jue|gui", "fu", "chen", "jian|yan", "fang|pang", "zhi", "qi|ta", "yue", \
    "ba|pa|pao", "ji|qi", "ti|yue", "qiang", "chi|tuo", "tai", "yi", "chen|jian|nian", "ling", "mei", \
    "ba|bei", "die|tu", "ku", "tuo", "jia", "ci|zi", "bo|pao", "qia", "zhu", "qu|ju", \
    "zhan|die|tie", "zhi", "fu", "ban|pan", "qu|ju", "shan", "bi|bo|po", "ni", "ju", "li|luo", \
    "gen", "yi", "ji", "duo|dai", "sun|xian", "jiao|qiao", "duo", "zhu|chu", "quan|zun", "ku|kua", \
    "zhuai|shi", "gui", "qiang|qiong", "kui|xie", "xiang", "die|chi", "lu|luo", "pian|beng", "zhi", "jia|jie", \
    "diao|tao|tiao", "cai", "jian", "da", "qiao", "bi", "xian", "duo", "ji", "ju|qu", \
    "ji", "shu|chou", "tu|duo", "cu|chu", "jing|keng", "nie", "qiao|xiao", "bu", "chi|xue", "cun|zun", \
    "mu", "shu", "lang|liang", "yong", "jiao", "chou", "qiao", "meo", "ta", "jian", \
    "qi|ji", "wo|wei", "cu|wei", "chuo|diao|tiao|zhuo", "jie", "ji|qi", "nie", "ju", "nie", "lun", \
    "lu", "cheng|leng", "huai", "ju", "chi", "wan|wo", "juan|quan", "die|ti", "bo|pou", "cu|zu", \
    "qie", "qi|ji", "cu|di", "zong", "cai|kui", "zong", "pan|peng", "zhi", "zheng", "dian|die", \
    "zhi", "yu|yao", "chuo|duo", "dun", "chun|chuan", "yong", "zhong", "di|chi", "zha", "chen", \
    "chuai|chuan|duan|shuan", "jian", "gua|tuo", "shang|tang", "ju", "fu|bi", "cu|zu", "die", "pian", "rou", \
    "na|nuo|re", "di|ti", "cha|zha", "tui", "jian", "dao", "cuo", "qi|xi", "ta", "qiang", \
    "nian|zhan", "dian", "ti|di", "ji", "nie", "liang|man|pan", "liu", "zan|can", "bi", "chong", \
    "lu", "liao", "cu", "tang", "dai|die", "su", "xi", "kui", "ji", "zhi|zhuo", \
    "qiang", "di|zhi", "liang|man|pan", "zong", "lian", "beng|cheng", "zao", "ran|nian", "bie", "tui", \
    "ju", "deng", "ceng", "xian", "fan", "chu", "zhong|chong", "dun|cun", "bo", "cu|jiu", \
    "cu", "gui|jue", "jue", "lin", "ta", "qiao", "qiao|jiao", "pu", "liao", "dun", \
    "cuan", "guan", "zao", "da", "bi", "bi", "zhu|zhuo", "ju", "chu|chuo", "qiao", \
    "dun", "chou", "ji", "wu", "yue|ti", "nian", "lin", "lie", "zhi", "li|yue|luo", \
    "zhi", "chan|zhan", "chu", "duan", "wei", "long", "lin", "xian", "wei", "cuo|zuan", \
    "lan", "xie", "rang", "xie|sa", "nie", "ta", "qu", "ji|qi", "cuan", "zuan|cuo", \
    "xi", "kui", "jue|qi", "lin", "juan|shen", "gong", "dan", "fen", "qu", "ti", \
    "duo", "duo", "gong", "lang", "ren", "luo", "ai", "ji", "ju", "tang", \
    "kong", "uu", "yan", "mei", "kang", "qu", "lou|lv", "lao", "duo|tuo", "zhi", \
    "yan", "ti", "dao", "ying", "yu", "che|ju", "ya|zha|ga", "gui", "jun", "wei", \
    "yue", "xin|xian", "dai", "xuan|xian", "fan|gui", "ren", "shan", "kuang", "shu", "tun", \
    "chen|qi", "tai|dai", "e", "na", "qi", "mao", "ruan", "kuang", "qian", "zhuan", \
    "hong", "hu", "gou|qu", "kuang", "di|chi", "ling", "dai", "ao", "zhen", "fan", \
    "kuang", "yang", "peng", "bei", "gu", "gu", "pao", "zhu", "fu|rong", "e", \
    "ba", "zhou", "zhi", "diao|yao", "ke", "die|yi|zhe", "qing|zhi", "shi", "peng|ping", "er", \
    "gong", "ju", "jiao|jue|xiao", "guang", "he|lu|ya", "kai", "chun|quan", "zhou", "zai|zi", "zhi", \
    "she", "liang", "yu", "shao", "you", "huan|wan", "qun|yin", "zhe", "wan", "fu", \
    "qing", "zhou", "ni|yi", "ling|leng", "zhe", "zhan", "liang", "zi", "hui", "wang", \
    "chuo", "guo|hua", "kan", "yi", "peng", "qian", "gun", "nian", "peng|ping", "guan", \
    "bei", "lun", "pai", "liang", "er|ruan", "rou", "ji", "yang", "xian|kan", "chuan", \
    "cou", "shun|chun", "ge|ya", "you", "hong", "shu", "fu|bu", "zi", "fu", "wen|yun", \
    "fan|ben", "nian|zhan", "yu", "wen|yun", "tao|kan", "gu", "zhen", "xia|he", "yuan", "lu", \
    "xiao|jiao", "chao", "zhuan|zhuai", "wei", "hun|xuan", "xue", "zhe", "jiao", "zhan", "bu", \
    "lao|liao", "fen", "fan", "lin", "ge", "se", "kan", "huan", "yi", "ji", \
    "zhui", "er", "yu", "jian", "hong", "lei", "pei", "li", "li", "lu", \
    "lin", "che|ju", "ya|zha|ga", "gui", "han|xian|xuan", "dai", "ren", "zhuan|zhuai", "e", "lun", \
    "ruan", "hong", "gu", "ke", "lu", "zhou", "zhi", "die|yi|zhe", "hu", "zhen", \
    "li", "diao|yao", "qing", "shi", "zai|zi", "zhi", "jiao", "zhou", "chun|quan", "he|lu|ya", \
    "jiao|jue|xiao", "zhe", "fu", "liang", "nian", "bei", "hui", "gun", "wang", "liang", \
    "chuo", "zi", "cou", "fu", "ji", "wen|yun", "shu", "pei", "yuan", "he|xia", \
    "zhan|nian", "lu", "zhe", "lin", "xin", "gu", "ci", "ci", "pi|bi", "zui", \
    "bian", "la", "la", "ci", "xue|yi", "ban|bian", "ban|bian", "ban|bian", "ban|bian|pian", "uu", \
    "bian", "ban", "ci", "bian", "bian|pian", "chen", "ru", "nong", "nong", "zhen", \
    "zou|chuo", "zou", "yi", "reng", "bian", "bian|dao", "shi", "ru", "liao", "da|ta|ti", \
    "chan", "gan", "qian", "yu", "yu", "qi", "xun", "yi|tuo", "guo", "mai", \
    "qi", "bi|za", "wang|kuang|guang", "tu", "zhun", "ying", "ti|da", "yun", "jin", "hang|xiang", \
    "ya", "fan", "wu", "ti|da", "e", "huan|hai", "zhe|zhei", "zhong", "jin", "yuan", \
    "hui|wei", "lian", "chi|zhi", "che", "ni|chi", "tiao", "chi|zhi|li", "tuo|yi", "jiong", "jia|xie", \
    "chen|zhen", "dai", "er", "di", "po|pai", "zhu|wang", "da|die|yi", "ze", "tao", "shu", \
    "tuo|yi", "keop|qu", "jing", "hui", "dong", "you", "mi", "beng|peng", "ji", "nai", \
    "yi", "jie", "dui|tui|zhui", "lie", "xun", "tui", "song", "kuo|shi", "tao", "feng|pang", \
    "hou", "ni", "dun", "jiong", "xuan|shua|suan", "xun", "bu", "you", "xiao", "qiu", \
    "shu|tou", "di|tun|zhou|zhu", "qiu", "di", "di", "tu", "jing", "ti", "dou|qi|tou|zhu", "yi|si", \
    "zhe|zhei", "tong", "guang|kuang", "wu", "shi", "cheng|ying", "su", "cao|zao", "qun|suo|xun", "feng|pang|peng", \
    "lian", "suo", "hui", "li", "gu", "lai", "ben", "cuo", "zhu|jue", "beng|peng", \
    "huan", "dai|di", "dai|lu", "you", "zhou", "jin", "yu", "chuo", "kui", "wei", \
    "ti", "yi", "da|ta", "yuan", "luo", "bi", "nuo", "dou|yu", "dang|tang", "sui", \
    "dun|qun|xun", "sui", "an|yan", "chuan", "chi", "ti", "ou|yong|yu", "shi", "zhen", "you", \
    "yun", "e", "bian", "guo|huo", "e", "xia", "huang", "qiu", "dao", "da|ta", \
    "hui|wei", "nan", "yi|wei", "gou", "yao", "chou", "liu", "xun", "ta", "di|shi", \
    "chi|xi|zhi", "yuan", "su", "ta", "qian", "hweong|ma", "yao", "guan", "zhang", "ao", \
    "di|shi|ti|zhe", "ca", "chi", "su", "zao", "zhe", "dun", "shi|di", "lou", "chi|zhi", \
    "cuo", "lin", "zun", "rao", "qian", "xuan|suan|shua", "yu", "yi|wei", "e", "liao", \
    "ju|qu", "shi", "bi", "yao", "mai", "xie", "sui", "huan|hai|xuan", "zhan", "teng", \
    "er", "miao", "bian", "bian", "la|lie", "li|chi", "yuan", "yao|you", "luo", "li", \
    "e|yi", "ting", "deng|shan", "qi", "yong", "shan", "han", "yu", "mang", "ru|fu", \
    "qiong", "wan", "kuang|kuo", "fu", "hang|kang", "bin", "fang", "geng|xing", "na|nei|ne", "xin", \
    "shen", "bang", "yuan", "cun", "huo", "xie|ye", "bang", "wu", "ju", "you", \
    "han", "tai", "qiu", "bi|bian", "pi", "bing", "shao", "bei", "wa", "di", \
    "ju|zou", "qiu|ye", "lin", "kuang", "gui", "zhu", "shi", "ku", "yu", "hai|gai", \
    "he|xia", "xi|qie", "ji|zhi", "ji", "xun|huan", "hou", "geng|xing", "jiao", "xi", "gui", \
    "fu|na|nuo", "lang", "jia", "kuai", "zheng", "lang", "yun", "yan", "cheng", "dou", \
    "xi|chi", "lv", "fu", "wu|yu", "fu", "gao", "hao|shi", "lang", "jia", "geng", \
    "jun", "cheng|ying", "bo", "xi", "qu|ju", "li|zhi", "yun", "bu|pou", "ao|xiao", "qi", \
    "pi", "qing", "guo", "zhou", "tan", "zou|ju", "ping", "lai|lei", "ni", "chen|lan", \
    "chui|you", "bu|pou", "xiang", "dan|duo", "ju", "yong", "qiao", "yi", "dou|du", "yan", \
    "mei", "ruo", "bei", "e", "shu", "juan", "yu", "yun", "hou", "kui", \
    "xiang", "xiang", "sou", "tang", "ming", "xi", "ru", "chu", "jin|zi", "zou|ju", \
    "ye", "wu", "xiang", "yun", "hao|jiao|qiao", "yong", "bi", "mao", "chao", "lu|fu", \
    "liao", "yin", "zhuan", "hu", "qiao", "yan", "zhang", "man|wan", "qiao", "xu", \
    "deng", "bi", "xun", "bi", "zeng", "wei", "zheng", "mao", "shan", "lin", \
    "pan|pi|po", "dan|duo", "meng", "ye", "cao|sao", "kuai", "feng", "meng", "ju|zou", "kuang|kuo", \
    "lian", "zan|cuo", "chan", "you", "ji|qi", "yan", "chan", "zan|cuo", "ling", "quan|huan", \
    "xi", "feng", "zan|cuo", "li|zhi", "you", "ding", "qiu", "zhuo", "pei", "zhou", \
    "yi", "gan|hang", "yu", "jiu", "yin|yan", "zui", "mao", "dan|zhen", "xu", "dou", \
    "zhen", "fen", "yuan", "fu", "yun", "tai", "tian", "qia", "dou|tuo", "zuo|cu", \
    "han", "gu", "su", "po|fa", "chou", "zai|zui", "ming", "lao|lu", "chuo", "chou", \
    "you", "chong|dong|tong", "zhi", "xian", "jiang", "cheng", "yin", "tu", "jiao", "mei", \
    "ku", "suan", "lei", "pu", "fu|zui", "hai", "yan", "shi|shai", "niang", "wei|zhui", \
    "lu", "lan", "yan|ang", "tao", "pei", "zhan", "chun", "tan|dan", "zui", "zhui", \
    "cu|zuo", "kun", "ti", "xian|jian", "du", "hu", "xu", "cheng|jing|xing", "tan", "chou|qiu", \
    "chun", "yun", "fa", "ke", "sou", "mi", "chuo|quan", "chou", "cuo", "yun", \
    "yong", "ang", "zha", "hai", "tang", "jiang", "piao", "chan|chen", "ou|yu", "li", \
    "zao", "lao", "yi", "jiang", "bu", "jiao|qiao|zhan", "xi", "tan", "fa|po", "nong", \
    "yi|shi", "li", "ju", "lian|yan", "yi|ai", "niang", "ru", "xun", "chou|shou", "yan", \
    "ling", "mi", "mi", "niang", "xin", "jiao", "shi|shai", "mi", "yan", "bian", \
    "cai", "shi", "you", "shi|yi", "shi|yi", "li", "zhong|chong", "shu|ye", "liang", "li|xi", \
    "jin", "jin", "ga|qiu", "yi", "liao", "dao", "zhao", "ding|ling", "po", "qiu", \
    "he|ba", "fu", "zhen", "zhi", "ba", "luan", "fu", "nai", "diao", "shan|xian", \
    "qiao|jiao", "kou", "chuan", "zi", "fan", "hua|yu", "wu|hua", "gan|han", "gang", "qi", \
    "mang", "ren|jian|ri", "di", "si", "xi", "yi", "chai|cha", "shi|yi", "tu", "xi", \
    "nv", "qian", "qiu", "jian", "pi|zhao", "ya|ye", "jin|yin", "ba|pa", "fang", "chen|qin", \
    "xing|jian", "dou|tou", "yao|yue", "zhong|qian", "fu", "pi|bu", "na|rui", "qin|xin", "e", "jue", \
    "dun", "gou", "yin", "han|qian", "ban", "xi|sa", "ren", "chao|miao", "chou|niu", "fen", \
    "dui|yun", "yi", "qin", "bi|pi", "guo", "hong", "yin", "jun", "diao", "yi", \
    "zhong", "xi", "gai", "ri", "huo", "tai", "kang", "yuan", "lu", "ngag", \
    "wen", "duo", "zi", "ni", "tu", "shi", "min", "gu|pi", "ke", "ling", \
    "bing", "si|ci", "gu|hu", "bo", "pi", "yu", "si", "zuo", "bu", "you|zhou", \
    "dian|tian", "ge|jia", "zhen", "shi", "shi|zu", "tie|zhi", "ju", "zuan|qian|chan", "yi|shi", "tuo|ta", \
    "xuan", "zhao", "bao|pao", "he", "bi|se", "sheng", "zu|ju|chu", "shi|zu", "bo", "zhu", \
    "chi", "za", "po", "tong", "an|qian", "fu", "zhai", "liu|mao", "qian|yan", "fu", \
    "li", "yue", "pi", "yang", "ban", "bo", "jie", "gou|qu", "shu|xu", "zheng", \
    "mu", "ni|nie|xi", "nie|xi", "di", "jia", "mu", "tan", "huan|shen", "yi", "si", \
    "kuang", "ka", "bei", "jian", "zhuo|tong", "xing", "hong", "jiao", "chi", "er|keng", \
    "ge|luo", "bing|ping", "shi", "mou|mao", "jia|ha", "yin", "jun", "zhou", "chong", "xiang|jiong", \
    "tong", "mo", "lei", "ji", "si|yu", "xu|hui", "ren", "zun", "zhi", "qiong", \
    "shan|shuo", "chi|li", "xi|xian", "jian|xing", "quan", "pi", "yi|tie", "zhu", "hou|xiang", "ming", \
    "kua", "yao|tiao|diao", "kuo|gua|tian|xian", "xian", "xiu", "jun", "cha", "lao", "ji", "pi", \
    "ru", "mi", "yi", "yin", "guang", "an", "diu", "you", "se", "kao", \
    "jian|qian", "luan", "si", "ngai", "diao", "han", "dui|rui|yue", "zhi|shi", "keng", "qiu", \
    "xiao", "zhe|nie", "xiu|you", "zang", "ti", "cuo", "gua", "gong|hong", "yong|zhong", "dou|tou", \
    "lv", "mei|meng", "lang", "wan|jian", "xin|zi", "jun|yun", "bei", "wu", "su", "yu", \
    "chan", "ting|ding", "bo", "han", "jia", "hong", "cuan|juan", "feng", "chan", "wan", \
    "zhi", "si|tuo", "juan|xuan", "wu|hua", "wu|yu", "tiao", "kuang", "chuo|zhuo", "lue", "jing|xiang|xing", \
    "jin|qian|qin", "shen", "han", "lue", "ye", "chu|ju", "zeng", "ju", "xian", "e|tie", \
    "mang", "pu", "li", "pan", "dui|rui|yue", "cheng", "gao", "li", "te", "bing", \
    "zhu", "zhen", "tu", "liu", "zui|nie", "ju", "chang", "wan|yuan", "jian", "gang", \
    "diao", "tao", "chang|shang", "lun|fen", "guo|ke|kua", "ling", "pi", "lu", "li", "qing|qiang", \
    "fu|pei|pou", "juan", "min", "zu|zui", "peng|beng", "an", "bei|bi|pi", "xian|qian", "ya", "zhui", \
    "lei|li", "a|ke", "kong", "ta", "kun|gun", "du", "wei|nei", "chui", "zi", "zheng", \
    "ben", "nie", "zong", "chun|dui", "tan|xian|tian", "ding", "qi|yi", "jian|qian", "zhui|chuo", "ji", \
    "yu", "jin", "guan", "mao", "chang", "tian|tun", "ti|xi", "lian|jian", "diao|tao", "gu", \
    "cuo|cu", "shu", "zhen", "lu|lv", "meng", "lu", "hua", "biao", "ga", "lai", \
    "ken", "fang", "wu", "nai", "wan|jian", "zan", "hu", "de", "xian", "uu", \
    "huo", "liang", "fa", "men", "jie|kai", "ying|yang", "chi|di", "lian|jian", "guo", "xian", \
    "du", "tu", "wei", "zong|wan", "fu", "rou", "ji", "e", "jun", "chen|zhen", \
    "ti", "zha", "hu", "yang", "duan", "xia", "yu", "keng", "sheng", "huang", \
    "wei", "fu", "zhao", "cha", "qie", "shi", "hong", "kui", "nuo|tian", "mou", \
    "qiao", "qiao", "hou", "tou", "zong", "huan", "ye|xie", "min", "jian", "duan", \
    "jian", "si|song", "kui", "hu", "xuan", "zhe|du", "jie", "qian|zhen", "bian", "zhong", \
    "zi", "xiu", "ye", "mei", "pai", "ai", "gai", "qian", "mei", "suo|cha", \
    "ta|da", "pang|bang", "xia", "lian", "suo|se", "kai", "liu", "yao|zu", "ta|ye", "hao|nou", \
    "weng", "rong", "tang", "suo", "qiang|cheng", "ge|li", "shuo", "chui|dui", "bo", "pan", \
    "da|sa", "bi|pi", "sang", "gang", "zi", "wu", "jiong|ying", "huang", "tiao", "liu", \
    "kai", "sun", "se|sha|shi", "sou", "wan|jian", "gao|hao", "tian|zhen", "tian|zhen", "lang|luo", "yi", \
    "yuan", "tang", "nie", "xi", "jia", "ge", "ma", "juan", "song|ka", "zu|ha", \
    "suo", "uu", "feng", "wen", "na", "lu", "suo", "kou", "zu|chuo", "tuan", \
    "xiu", "guan", "xuan", "lian", "sou|shou", "ao", "man", "mo", "luo", "bi", \
    "wei", "liao|liu", "di", "can|qiao|san", "zong", "yi", "ao|lu", "ao|biao", "keng", "qiang", \
    "cui", "qi", "chang", "tang", "man", "yong", "chan", "feng", "jing", "biao", \
    "shu", "lou|lv", "xiu", "cong", "long", "zan", "zan|jian", "cao", "li", "xia", \
    "xi", "kang", "shuang", "beng", "zhang", "qian", "cheng", "lu", "hua", "ji", \
    "pu", "sui|hui", "qiang", "po", "lin", "se", "xiu", "san|sa|xian", "cheng", "kui", \
    "si", "liu", "nao", "huang", "pie", "sui", "fan", "qiao", "quan", "yang", \
    "tang", "xiang", "yu|jue", "jiao", "zun", "liao", "qie|qi", "lao", "dun|dui", "tan|xin", \
    "zan", "ji|qi", "jian", "zhong", "deng", "ya", "ying", "dun|dui", "jue", "hao|nou", \
    "ti|zan", "pu", "tie|die|te", "uu", "zhang", "ding", "shan", "kai", "jian", "fei", \
    "sui", "lu", "juan", "hui", "yu", "lian", "zhuo", "sao|qiao", "jian|qian", "shu|zhuo", \
    "lei", "bi|bei", "die|tie", "huan|xuan", "ye|xie", "duo", "guo", "dang|tang|cheng", "ju|qu", "ben|fen", \
    "da", "bei|bi", "yi", "ai", "zong", "xun", "diao", "zhu", "heng", "zhui", \
    "ji", "ni|nie", "he", "huo", "qing", "bin", "ying", "kui", "ning", "xu|ru", \
    "jian", "jian", "qian", "cha", "zhi", "mie|mi", "li", "lei", "ji", "zuan", \
    "kuang", "shang", "peng", "la", "du", "li|shuo|yue", "chuo", "lv", "biao", "bao", \
    "lu", "xian|zhi", "kuan", "long", "e", "lu", "xin|xun", "jian", "lan", "bo", \
    "qian|jian", "yao|yue", "chan", "xiang|rang", "jian", "xi", "guan", "cang", "nie", "lei", \
    "cuan", "qu", "pan", "luo", "zuan", "luan", "zao|zuo", "nie|yi", "jue", "tang", \
    "zhu", "lan", "jin|pianpang", "ga|qiu", "yi", "zhen", "ding|ling", "zhao", "po", "liao", \
    "tu", "qian", "chuan", "shan|xian", "xi|sa", "fan", "diao", "men", "nv", "yang", \
    "cha|chai", "xing|jian", "gai", "bu|pi", "tai", "ju", "dun", "chao", "zhong", "na|rui", \
    "bei", "gang", "ban", "han|qian", "yao|yue", "qin", "jun", "wu", "gou", "kang", \
    "fang", "huo", "dou|tou", "chou|niu", "ba|pa", "yu", "jian|qian", "zheng", "an|qian", "gu|hu", \
    "bo", "ke", "po", "bu", "bo", "yue", "zuan", "mu", "tan", "ge|jia", \
    "tian|dian", "you|zhou", "tie|zhi", "bo", "ling", "li|shuo|yue", "qian|yan", "liu|mao", "bao|pao", "shi", \
    "xuan", "ta|tuo", "bi|se", "ni", "pi", "duo", "xing", "kao", "lao", "er|keng", \
    "mang", "ya", "you", "cheng", "jia", "ye", "nao", "zhi", "dang|cheng", "tong", \
    "lv", "diao", "yin", "kai", "zha", "zhu", "xian|xi", "ting|ding", "diu", "kuo|gua|tian|xian", \
    "hua", "quan", "se|sha|shi", "ha|ke", "tiao|yao|diao", "ge|luo", "ming", "zheng", "se", "jiao", \
    "yi", "chan", "chong", "tang", "an", "yin", "ru", "zhu", "lao", "pu", \
    "wu|yu", "lai", "te", "lian", "keng", "xiao", "suo", "li", "zeng", "chu|ju", \
    "guo", "gao", "e|tie", "xiu|you", "cuo", "lue", "feng", "xin|zi", "liu", "kai", \
    "jian", "dui|rui|yue", "ti", "lang", "jin|qian|qin", "ju", "a", "qiang", "du|duo|zhe", "nuo|tian", \
    "cu|cuo|xi", "mao", "ben", "qi|yi", "de", "guo|ke|kua", "gun|kun", "chang", "ti|xi", "gu", \
    "luo", "chui", "zhui", "jin", "zhi", "xian", "juan", "huo", "fu|pei|pou", "tan|xian", \
    "ding", "jian", "ju", "meng", "zi", "qie", "ying|yang", "jie|kai", "qiang", "si|song", \
    "e", "cha", "qiao", "zhong", "duan", "sou", "huang", "huan", "ai", "du", \
    "mei", "lou|lv", "zi", "fei", "mei", "mo", "tian|zhen", "bo", "ge|li", "nie", \
    "tang", "juan", "nie", "na", "liu", "gao|hao", "bang|pang", "yi", "jia", "bin", \
    "rong", "biao", "tang", "man", "luo", "beng", "yong", "jing", "di", "zu|chuo", \
    "xuan", "liao|liu", "tan|chan|xin", "jue", "liao", "pu", "lu", "dun|dui", "lan|lian", "pu", \
    "chuan|cuan", "qiang", "deng", "huo", "lei", "huan|xuan", "shu|zhuo", "lian", "yi", "cha", \
    "biao", "la", "chan", "rang|xiang", "chang|zhang", "chang|zhang", "jiu", "ao", "die", "jue", \
    "liao", "mi|ni", "chang|zhang", "men", "ma", "shuan", "shan", "shan|huo", "men", "yan", \
    "bi", "han|bi", "bi", "shan", "kai|qian", "kang", "beng", "hong", "run", "san", \
    "xian", "xian|jian", "jian", "min", "xia", "shui", "dou", "zha|ya", "nao", "zhan", \
    "peng", "xia|e", "ling", "bian|guan", "bi", "run", "he|ai", "guan|wan", "ge", "he|ge", \
    "fa", "chu", "hong|xiang", "gui", "min", "seo", "kun", "lang|liang", "lv", "ting", \
    "sha", "ju", "yue", "yue", "chan", "qu", "lin", "chang|tang", "sha|shai", "kun", \
    "yan", "wen", "yan", "yu|e|yan", "hun", "yu", "wen", "xiang|hong", "bao", "juan|xiang|hong", \
    "qu", "yao", "wen", "pan|ban", "an|yin", "wei", "yin", "kuo", "jue|kui|que", "lan", \
    "du|she", "quan", "phdeng", "tian", "nie", "ta", "kai", "he", "jue|que", "chuang|chen", \
    "guan|wan", "dou", "qi", "kui", "tang|chang", "guan|wan", "piao", "kan|han", "xi|se", "hui", \
    "chan", "pi", "dang|tang", "huan", "ta", "wen", "uu", "men", "shuan", "shan", \
    "yan", "han|bi", "bi", "wen", "chen|chuang", "run", "wei", "xian", "hong", "jian", \
    "min", "kang", "men", "ge|ya|zha", "nao", "gui", "wen", "ta", "min", "lv", \
    "kai", "fa", "ge", "ai|gai|hai|he", "kun", "jiu", "yue", "lang|liang", "du|she", "yu", \
    "yan", "chang|tang", "he|xi", "wen", "hun", "yan", "yan|e", "chan", "lan", "qu", \
    "hui", "kuo", "jue|kui|que", "he", "tian", "ta", "jue|que", "kan|han", "huan", "fu", \
    "fu|pianpang", "le", "dui", "xin", "qian", "wei|wu", "gai|yi", "yi|tuo|zhi", "yin", "yang", \
    "dou", "e|ai", "sheng", "ban", "pei", "keng|gang", "yan|yun", "ruan|yuan", "zhi", "pi", \
    "jing", "fang", "yang", "lin|yin", "zhen", "jie", "cheng", "e|ai", "qu", "di", \
    "zhu|zu", "zuo", "yan|dian", "ling|lin", "a|e", "duo|tuo", "tuo|yi|zhi", "po|pi|bei", "bing", "bu|fu", \
    "ji", "lu|liu", "long", "chen", "jing|xing", "duo", "lou", "mo", "jiang|xiang", "shu", \
    "duo|sui", "wen|xian", "er", "gui", "yu", "gai", "shan", "jun", "qiao", "jing|xing", \
    "chun", "fu|wu", "bi", "xia", "shan", "sheng", "de|zhi", "pu|bu", "dou", "yuan", \
    "zhen", "chu|shu|zhu", "xian", "dao", "nie", "yuan|yun", "jian|xian|yan", "pei", "fei|pei", "zhe|zou", \
    "qi|yi", "dui", "lun", "an|yan|yin", "ju", "chui", "chen|zhen", "bi|pi", "ling", "tao|yao", \
    "xian", "lu|liu", "sheng", "xian", "yin", "du|zhu", "yang", "reng|er", "xia", "chong", \
    "yan", "an|yin", "yu|shu|yao", "di", "yu", "long", "wei", "wei", "nie", "dui|zhui", \
    "duo|sui|tuo", "an", "huang", "jie", "sui", "yin", "ai|gai", "yan", "duo|hui", "ge|ji|rong", \
    "yuan|yun", "wu", "wei|kui", "ai|e", "xi", "tang", "ji", "zhang", "dao", "ao", \
    "xi", "yin", "sa", "rao", "lin", "tui", "deng", "pi", "sui|zhui", "sui", \
    "yu", "xian|jian", "fen", "ni", "er", "ji", "dao", "xi|xie", "yin", "zhi", \
    "duo|hui", "long", "xi", "dai|di|li|yi", "li", "li", "cui|wei|zhui", "que|he", "huo|zhi", "sun", \
    "juan|jun", "nan|nuo", "yi", "que|qiao", "yan", "qin", "jie|qian", "xiong", "ya", "ji", \
    "gu|hu", "huan", "kai|si|yi|zhi", "gou", "jun|juan", "ci", "yong", "ju", "chu|ju", "hu", \
    "za", "luo", "yu", "chou", "diao", "sui", "han", "huo|wo", "shuang", "huan|guan", \
    "chu|ju", "za", "yong", "ji", "xi|gui", "chou", "liu", "chi|li", "nan|nuo", "xue", \
    "za", "ji", "ji", "yu", "xu|yu", "xue", "na", "fou", "xi|se", "mu", \
    "wen", "fen", "fang|pang", "yun", "li", "chi", "yang", "lian|ling", "lei", "an", \
    "bao", "wu|meng", "dian", "dang", "hu", "wu", "diao", "nuo|ruan|xu", "ji", "mu", \
    "chen", "xiao", "sha|zha", "ting", "shen|zhen", "pei", "mei", "ling", "qi", "zhou", \
    "he|huo|suo", "sha", "fei", "hong", "zhan", "yin", "ni", "zhu|shu", "tun", "lin", \
    "ling", "dong", "ying|yang", "wu|meng", "ling", "shuang", "ling", "xia", "hong", "yin", \
    "mai", "mai", "yun", "liu", "meng", "bin", "meng|wu", "wei", "kuo", "yin", \
    "xi", "yi", "ai", "dan", "teng", "san|xian", "yu", "lu|lou", "long", "dai", \
    "ji", "pang", "yang", "ba|po", "pi", "wei", "uu", "xi", "ji", "li|mai", \
    "mao|meng|wu", "meng", "lei", "li", "sui|huo", "ai", "fei", "dai", "long|ling", "ling", \
    "ai|yi", "feng", "li", "bao", "he", "he", "he", "bing", "jing|qing", "jing|qing", \
    "liang|jing", "tian", "zhen", "jing", "cheng", "jing|qing", "jing", "liang|jing", "dian", "jing", \
    "tian", "fei", "fei", "kao", "ma|mi", "mian", "mian", "bao", "yan|ye", "tian|mian", \
    "hui", "yan|ye", "ge|ji", "ding", "cha", "qian|jian", "ren", "di", "du", "wu", \
    "ren", "qin", "jin", "xue", "niu", "ba", "yin", "sa", "na", "mo|wa", \
    "zu", "da", "ban", "yi|xie", "yao", "tao", "bei|bi", "jie", "hong", "pao", \
    "yang", "bing", "yin", "ge|sa|ta", "tao", "ji|jie", "wa|xie", "an", "an", "hen", \
    "gong", "qia", "da|ta", "jue|qiao", "ting", "man|men", "bian|ying", "sui", "tiao", "qiao|shao", \
    "juan|xuan", "kong", "beng", "ta", "shang|zhang", "bi|bing", "kuo", "ju|qiong|qu", "la", "die|xie|zha", \
    "rou", "bang", "eng", "qiu", "qiu", "he|she", "qiao|shao", "mou|mu", "ju|qu", "jian", \
    "bian", "di", "jian", "on", "tao", "gou", "ta", "bai|bei|bu|fu", "xie", "pan", \
    "ge", "bi|bing", "kuo", "tang", "lou", "gui|hui", "jue|qiao", "xue", "ji", "jian", \
    "jiang", "chan", "ta|da", "hu", "xian", "qian", "du", "wa", "jian", "lan", \
    "hui|wei", "ren", "fu", "wa|mei", "juan|quan", "ge", "wei", "qiao|shao", "han", "chang", \
    "kuo", "rou|ruo", "yun", "she", "wei", "ge", "bai|fu", "tao", "gou|bei", "wen|yun", \
    "gao", "bi", "xue|wei", "sui|hui", "du", "wa", "du", "hui|wei", "ren", "fu", \
    "han", "wei", "wen|yun", "tao", "jiu", "jiu", "xian", "xie", "xian", "ji", \
    "yin", "za", "yun", "shao", "le", "peng", "huang|ying", "ying", "yun", "peng", \
    "an", "yin", "xiang", "hu", "ye|xie", "ding", "kui|qing", "qiu|kui", "xiang", "shun", \
    "han|an", "xu", "yi", "xu", "gu|e", "rong|song", "kui", "ken|qi", "gang|hang", "yu", \
    "wan|kun", "ban|fen", "dun|du", "di", "dan|dian", "pan", "pi|po", "ling", "che", "jing", \
    "lei", "he|ge|han", "qiao", "an|e", "e", "wei", "xie|jie", "kuo", "shen", "yi", \
    "yi", "hai|ke", "dui", "yu|bian", "ping", "lei", "fu|tiao", "jia", "tou", "pou|hui", \
    "kui", "jia", "luo", "ting", "cheng", "jing|ying", "yun", "hu", "han", "jing|geng", \
    "tui", "tui", "bin|pin", "lai", "tui", "zi", "zi", "chui", "ding", "lai", \
    "tan|shan", "han", "qian", "ke|kuan", "zu|cui", "xian|jiong", "qin", "yi", "sai", "di|ti", \
    "e", "e", "yan", "hun|wen", "yan|kan", "yu|yong", "zhuan", "ya|yan", "xian", "pi|xin", \
    "yi", "yuan", "sang", "dian|tian", "dian|tian", "jiang", "kua|kui", "lei", "lao", "piao", \
    "wai|zhuai", "man", "cu", "qiao|yao", "hao", "qiao", "gu", "xun", "yan|qin", "hui", \
    "zhan|chan|shan", "ru", "meng", "bin", "xian", "pin", "lu", "lin|lan", "nie", "quan", \
    "xie|ye", "ding", "kui|qing", "an|han", "xiang", "shun", "xu", "xu", "kun|wan", "gu", \
    "dun|du", "ken|qi", "ban|fen", "rong|song", "hang|gang", "yu", "lu", "ling", "pi|po", "jing|geng", \
    "jie|xie|jia", "jia", "ting", "he|ge", "ying", "jiong", "hai|ke", "yi", "bin|pin", "pou", \
    "tui", "han", "jing|ying", "ying", "ke|kuan", "ti|di", "yu|yong", "e", "zhuan", "ya|yan", \
    "e", "nie", "man", "dian|tian", "sang", "hao", "lei", "chan|zhan", "ru", "pin", \
    "quan", "feng", "diao|diu", "gua", "fu", "xia", "zhan", "biao|pao", "li|sa", "ba|fu", \
    "tai", "lie", "gua|ji", "xuan", "shao|xiao", "ju", "biao", "si", "wei", "yang", \
    "yao", "sou", "kai", "sou|sao", "fan", "liu", "xi", "liao|liu", "piao", "piao", \
    "liu", "biao", "biao", "biao", "liao", "biao", "se", "feng", "xiu", "feng", \
    "yang", "zhan", "biao|pao", "li|sa", "ju", "si", "sou", "yao", "liu", "piao", \
    "biao", "biao", "fei", "fan", "fei", "fei", "shi|si|yi", "shi", "can", "ji", \
    "ding", "si", "tuo", "gan|zhan", "sun", "xiang", "tun|zhun", "ren", "yu", "yang|juan", \
    "chi|shi", "yin", "fan", "fan", "can|sun", "yin", "zhu|tou", "yi|si", "zha|zuo|ze", "bi", \
    "jie", "tao", "bao", "ci", "tie", "si", "bao", "chi|shi", "duo", "hai", \
    "ren", "tian", "jiao", "he|jia", "bing", "yao", "tong", "ci", "xiang", "yang", \
    "juan", "er", "yan", "le", "xi", "can|sun", "bo", "nei", "e", "bu", \
    "jun", "dou", "su", "ye|yu", "xi|shi", "yao", "hun|kun", "guo", "chi|shi", "jian", \
    "zhui", "bing", "kan|xian", "bu", "ye", "dan|tan", "fei", "zhang", "wei|nei", "guan", \
    "e", "nuan", "hun|yun", "hu", "huang", "tie", "hui", "zhan|jian", "hou", "he|ai", \
    "tang|xing", "fen", "wei", "gu", "zha|cha", "song", "tang", "bo", "gao", "xi", \
    "kui", "liu", "sou", "tao|xian", "ye", "wen", "mo", "tang", "man", "bi", \
    "yu", "xiu", "jin", "san", "kui|tui", "xuan|zhuan", "shan", "xi|chi", "dan", "ye|yi", \
    "ji|qi", "rao", "cheng", "yong", "tao", "wei", "xiang", "zhan", "fen", "hai", \
    "meng", "yan", "mo", "chan", "xiang", "luo", "zan", "nang", "pianpang|shi", "ding", \
    "ji", "tuo", "tang|xing", "tun|zhun", "xi", "ren", "yu", "chi|shi", "fan", "yin", \
    "jian", "chi|shi", "bao", "si", "duo", "si|yi", "er", "rao", "xiang", "he|jia", \
    "le|ge", "jiao", "xi", "bing", "bo", "dou", "e", "ye|yu", "nei", "jun", \
    "guo", "hun|kun", "kan|xian", "guan", "zha|cha", "kui|tui", "gu", "sou", "chan", "ye", \
    "mo", "bo", "liu", "xiu", "jin", "man", "san", "xuan|zhuan", "nang", "shou", \
    "kui|qiu", "guo|xu", "xiang", "fen", "bo", "ni", "bi", "bo|po", "tu", "han", \
    "fei", "jian", "an", "ai", "bi|fu", "xian", "yun|wo", "xin", "fen", "pin", \
    "xin", "ma", "yu", "feng|ping", "han|qian", "di", "tuo|duo", "tuo|zhe", "chi", "xun", \
    "zhu", "zhi|shi", "pei", "jin|xin", "ri", "sa", "yun", "wen", "zhi", "dan", \
    "lv|lu", "you", "bo", "bao", "jue|kuai", "tuo|duo", "yi", "qu", "pu", "qu", \
    "jiong", "po", "zhao", "yuan", "pei|peng", "zhou", "ju", "zhu", "nu", "ju", \
    "pi", "zu|zang", "jia", "ling", "zhen", "tai|dai", "fu", "yang", "shi", "bi", \
    "tuo", "tuo", "si", "liu", "ma", "pian", "tao", "zhi", "rong", "teng", \
    "dong", "xun|xuan", "quan", "shen", "jiong", "er", "hai", "bo", "zhu", "yin", \
    "luo|jia", "zhou", "dan", "hai", "liu", "ju", "song", "qin", "mang", "liang|lang", \
    "han", "tu", "xuan", "tui", "jun", "e", "cheng", "xing", "si|ai", "lu", \
    "zhui", "zhou|dong", "she", "pian", "kun", "tao", "lai", "zong", "ke", "qi|ji", \
    "qi", "yan", "fei", "sao", "yan", "ge", "yao", "wu", "pian", "cong", \
    "pian", "qian", "fei", "huang", "qian", "huo", "yu", "ti", "quan", "xia", \
    "zong", "jue|kui", "rou", "si", "gua", "tuo", "gui|tui", "sou", "qian|jian", "cheng", \
    "zhi", "liu", "bang|peng", "teng", "xi", "cao", "du", "yan", "yuan", "zou|zhu", \
    "sao|xiao", "shan", "qi", "zhi|chi", "shuang", "lu", "xi", "luo", "zhang", "mo|ma", \
    "ao|yao", "can", "biao|piao", "cong", "qu", "bi", "zhi", "yu", "xu", "hua", \
    "bo", "su", "xiao", "lin", "zhan", "dun", "liu", "tuo", "ceng", "dian", \
    "jiao|ju|qiao|xiao", "tie", "yan", "luo", "zhan", "jing", "yi", "ye", "zhe|tuo", "pin", \
    "zhou", "yan", "long|zang", "lv", "teng", "xiang", "ji", "shuang", "ju", "xi", \
    "huan", "chi|li", "biao|piao", "ma", "yu", "tuo|duo", "xun", "chi", "qu", "ri", \
    "bo", "lv", "zang|zu", "shi", "si", "fu", "ju", "qu|zhou|zhu|zou", "zhu", "tuo", \
    "nu", "jia", "yi", "tai|dai", "xiao", "ma", "yin", "jiao|ju|qiao|xiao", "hua", "jia|luo", \
    "hai", "pian", "biao|piao", "chi|li", "cheng", "yan", "xing", "qin", "jun", "qi", \
    "qi|ji", "ke", "zhui", "zong", "su", "can", "pian", "zhi", "jue|kui", "sao|xiao", \
    "wu", "ao|yao", "liu", "jian|qian", "shan", "biao|piao", "luo", "cong", "chan|zhan", "zhou", \
    "ji", "shuang", "xiang", "gu", "wan|wei", "wan|wei", "wan|wei", "yu", "gan", "yi", \
    "ang|kang", "gu|tou", "jia|jie|xie", "bao", "bei", "ci|zhai", "ti", "di", "ku", "gai|hai", \
    "jiao|qiao", "hou", "kua", "ge", "tui", "geng", "pian", "bi", "ke|kua", "ge|qia", \
    "yu|ou", "sui", "lou", "bo|po", "xiao", "pang|bang", "jue|bo", "cuo|ci", "kuan", "bin", \
    "mo", "liao", "lou", "xiao", "du", "zang", "sui", "ti", "bin", "kuan", \
    "lu", "gao", "gao", "qiao", "kao", "qiao", "lao", "sao", "biao|piao", "kun", \
    "kun", "ti|di", "fang", "xiu", "ran", "mao", "dan", "kun", "bin", "fa", \
    "jie|tiao", "pi", "zi", "fa", "ran", "ti|di", "bao", "bi|po", "mao|meng|rou", "fei|fu", \
    "er", "rong|er", "qu", "gong", "xiu", "kuo|yue", "ji", "peng", "zhua", "shao", \
    "sha|suo", "ti", "li", "bin", "zong", "ti|di", "peng", "song", "zheng", "quan", \
    "zong", "shun", "jian", "chui|duo", "hu", "la", "jiu", "qi", "lian", "zhen", \
    "bin", "peng", "ma", "san", "man", "man", "seng", "xu", "lie", "qian", \
    "qian", "nang", "huan", "kuai|kuo", "ning", "bin", "lie", "rang|ning", "dou", "dou", \
    "nao", "xiang|hong", "he|xi", "dou", "han", "dou", "dou", "jiu", "chang", "yu", \
    "yu", "ge|li", "yan", "fu|li", "xin|qin", "gui", "zeng|zong", "liu", "gui|xie", "shang", \
    "ju|yu|zhou", "gui", "mei", "qi|ji", "qi", "ga", "kuai|kui", "hun", "ba", "po|bo|tuo", \
    "mei", "xu", "yan", "xiao", "liang", "yu|huo", "zhui|tui", "qi", "wang", "liang", \
    "wei", "gan", "chi", "piao", "bi", "mo", "qi", "xu", "chou", "yan", \
    "zhan", "yu", "dao", "ren", "ji|jie", "ba", "hong|gong", "tuo", "diao|di", "ji", \
    "xu|yu", "hua|e", "ji|e|qie", "sha|suo", "hang", "tun", "mo", "jie", "shen", "ban", \
    "yuan|wan", "bi|pi", "lu|lv", "wen", "hu", "lu", "shi|za", "fang", "fen", "na", \
    "you", "pian", "mo", "he|ge", "xia", "qu|xie", "han", "pi", "ling|lin", "tuo", \
    "ba|bo", "qiu", "ping", "fu", "bi", "ci|ji", "wei", "ju|qu", "diao", "bo|ba", \
    "you", "gun", "ju|pi", "nian", "xing|zheng", "tai", "bao|pao", "fu", "zha", "ju", \
    "gu", "shi", "dong", "chou|dai", "ta|die", "jie|qia", "shu", "hou", "xiang|zhen", "er", \
    "an", "wei", "zhao", "zhu", "yin", "lie", "luo|ge", "tong", "yi|ti", "yi|qi", \
    "bing|bi", "wei", "jiao", "ku", "gui|xie", "xian", "ge", "hui", "lao", "fu", \
    "kao", "xiu", "duo", "jun", "ti", "mian", "shao", "zha", "suo", "qin", \
    "yu", "nei", "zhe", "gun", "geng", "su", "wu", "qiu", "shan|shen", "pu|bu", \
    "huan", "tiao|chou", "li", "sha", "sha", "kao", "meng", "cheng", "li", "zou", \
    "xi", "yong", "shen", "zi", "qi", "qing|zheng", "xiang", "nei", "chun", "ji", \
    "diao", "qie", "gu", "zhou", "dong", "lai", "fei", "ni", "yi", "kun", \
    "lu", "ai|jiu", "chang", "jing", "lun", "ling", "zou", "li", "meng", "zong", \
    "zhi|ji", "nian", "hu", "yu", "di", "shi", "shen|can", "huan", "ti", "hou", \
    "xing|zheng", "zhu", "la", "zong", "ji|zei", "bian", "bian", "huan", "quan", "zei", \
    "wei", "wei", "yu", "chun", "rou", "die|qie", "huang", "lian", "yan", "qiu", \
    "qiu", "jian", "bi", "e", "yang", "fu", "sai|xi", "jian|gan", "xia", "wei|tuo", \
    "hu", "shi", "ruo", "xuan", "wen", "jian|qian", "hao", "wu", "fang|pang", "sao", \
    "liu", "ma", "shi", "shi", "guan|gun|kun", "zi", "teng", "die|ta", "yao", "ge|e", \
    "yong", "qian", "qi", "wen", "ruo", "shen", "lian", "ao", "le", "hui", \
    "min", "ji", "tiao", "qu", "jian", "shen|can", "man", "xi", "qiu", "biao", \
    "ji", "ji", "zhu", "jiang", "xiu|qiu", "zhuan|tuan", "yong", "zhang", "kang", "xue", \
    "bie", "yu", "qu", "xiang", "bo", "jiao", "xun", "su", "huang", "zun", \
    "shan|tuo", "shan", "fan", "jue|gui", "lin", "xun", "miao", "xi", "zeng", "xiang", \
    "fen", "guan", "hou", "kuai", "zei", "sao", "zhan|shan", "gan", "gui", "sheng|ying", \
    "li", "chang", "lei", "shu", "ai", "ru", "ji", "yu|xu", "hu", "shu", \
    "li", "la|lie", "li|lu", "mie", "zhen", "xiang", "e", "lu", "guan", "li", \
    "xian", "yu", "dao", "ji", "you", "tun", "lu|lv", "fang", "ba", "he|ge", \
    "ba|bo", "ping", "nian", "lu", "you", "zha", "chou|fu", "bo|ba", "bao|pao", "hou", \
    "ju|pi", "tai", "gui|xie", "jie|qia", "kao", "wei", "er", "tong", "zei", "hou", \
    "kuai", "ji", "jiao", "xian", "zha", "xiang|zhen", "xun", "geng", "li", "lian", \
    "jian", "li", "shi", "tiao", "gun", "sha", "huan", "jun", "ji|zei", "yong", \
    "qing|zheng", "ling", "qi", "zou", "fei", "kun", "chang", "gu", "ni", "nian", \
    "diao", "jing", "shen|can", "shi", "zi", "fen", "die|qie|zha", "bi", "chang", "ti", \
    "wen", "wei", "sai|xi", "e", "qiu", "fu", "huang", "quan", "jiang", "bian", \
    "sao", "ao", "qi", "die|ta", "guan|gun|kun", "yao", "fang|pang", "jian|qian", "le", "biao", \
    "xue", "bie", "man", "min", "yong", "wei", "xi", "jue|gui", "shan", "lin", \
    "zun", "hu", "gan", "li", "shan|zhan", "guan", "niao|diao", "yi", "fu", "li", \
    "jiu|qiu|zhi", "bu", "yan", "fu", "diao|zhao", "ji", "feng", "ru", "gan|han", "shi", \
    "feng", "ming", "bao", "yuan", "zhi|chi", "hu", "qin", "gui|fu", "fen|ban", "wen", \
    "qian|jian", "shi", "yu", "fou", "ao|yao", "jue|gui", "jue|gui", "pi", "huan", "zhen", \
    "bao", "yan", "ya", "zheng", "fang", "feng", "wen", "ou", "dai", "jia|ge", \
    "ru", "ling", "bi|mie", "fu", "tuo", "min|wen", "li", "bian", "zhi", "ge", \
    "yuan", "ci", "gou|qu", "xiao", "chi", "dan", "ju", "ao|yao", "gu", "zhong", \
    "yu", "yang", "yu", "ya", "die|tie|hu", "yu", "tian", "ying|xue", "dui", "wu", \
    "er", "gua", "ai", "zhi", "yan|e", "heng", "xiao", "jia", "lie", "zhu", \
    "yang|xiang", "yi|ti", "hong", "lu", "ru", "mou", "ge", "ren", "jiao|xiao", "xiu", \
    "zhou|diao", "chi", "luo|ge", "heng", "nian", "e", "luan", "jia", "ji", "tu", \
    "juan|huan", "tuo", "bu|pu", "wu", "juan", "yu", "bo", "jun", "jun", "bi", \
    "xi", "jun", "ju", "tu", "jing", "ti", "e", "e", "kuang", "hu|gu", \
    "wu", "shen", "lai|chi", "jiao", "pan", "lu", "pi", "shu", "fu", "an|ya", \
    "zhuo", "feng|peng", "qiu", "qian", "bei", "diao", "lu", "que", "jian", "ju", \
    "tu", "ya", "yuan", "qi", "li", "ye", "zhui", "kong", "duo", "kun", \
    "sheng", "qi", "jing", "yi", "yi", "qing|jing", "zi", "lai", "dong", "qi", \
    "chun|tuan", "geng", "ju", "qu|jue", "yi", "zun", "ji", "shu", "uu", "chi", \
    "miao", "rou", "an|ya", "qiu", "ti|chi", "hu", "ti|chi", "e", "jie", "mao", \
    "fu|bi", "chun", "tu", "yan", "he|jie", "yuan", "bian|pian", "kun", "mei", "hu", \
    "ying", "chuan|zhi", "wu", "ju", "dong", "qiang|cang", "fang", "hu|he", "ying", "yuan", \
    "xian", "weng", "shi", "he", "chu", "tang", "xia", "ruo", "liu", "ji", \
    "hu|gu", "jian|qian", "sun|xun", "han", "ci", "ci", "yi", "yao", "yan", "ji", \
    "li", "tian", "kou", "ti", "ti|si", "yi", "tu", "ma", "xiao", "gao", \
    "tian", "chen", "ji", "tuan", "zhe", "ao", "yao|xiao", "yi", "ou", "chi", \
    "zhi|zhe", "liu", "yong", "lv", "bi", "shuang", "zhuo", "yu", "wu", "jue", \
    "yin", "ti|tan", "si", "jiao", "yi", "hua", "bi", "ying", "su", "huang", \
    "fan", "jiao", "liao", "yan", "gao", "jiu", "xian", "xian", "tu", "mai", \
    "zun", "shu|yu", "ying", "lu", "tuan", "xian", "xue", "yi", "pi", "shu|zhu|chu", \
    "luo", "xi", "yi", "ji", "ze", "yu", "zhan", "ye", "yang", "bi|pi", \
    "ning", "hu", "mi", "ying", "mang|meng", "di", "yue", "yu", "lei", "bu", \
    "lu", "he", "long", "shuang", "yue", "ying", "huan|guan", "gou|qu", "li", "luan", \
    "niao|diao", "jiu|qiu|zhi", "ji", "yuan", "ming", "shi", "ou", "ya", "cang|qiang", "bao", \
    "zhen", "gu", "dong", "lu", "ya", "xiao", "yang", "ling", "chi", "gou|qu", \
    "yuan", "xue", "tuo", "si", "zhe|zhi", "er", "gua", "xiu", "heng", "zhou|diao", \
    "ge", "luan", "hong", "wu", "bo", "li", "juan", "hu|gu", "e", "yu", \
    "xian", "ti", "wu", "que", "miao", "an|ya", "kun", "bei", "feng|peng", "qian", \
    "chun|tuan", "geng", "yuan", "su", "hu", "he", "e", "hu|gu", "qiu", "ci", \
    "mei", "wu", "yi", "yao", "weng", "liu", "ji", "yi", "jian|qian", "he", \
    "yi", "ying", "zhe", "liu", "liao", "jiao", "jiu", "shu|yu", "lu", "huan", \
    "zhan", "ying", "hu", "mang|meng", "guan|huan|quan", "shuang", "lu", "jin", "ling", "jian", \
    "xian|jian", "cuo", "jian", "jian", "yan", "cuo", "lu|lv", "you", "cu", "ji", \
    "biao|pao", "cu", "pao", "cu|zhu", "jun|qun", "zhu", "jian", "mi", "mi", "yu", \
    "liu", "chen", "jun|qun", "lin", "ni", "qi", "lu", "jiu", "jun|qun", "jing", \
    "li|si", "xiang", "xian|yan", "jia", "mi", "li", "she", "zhang", "lin", "jing", \
    "qi", "ling", "yan", "cu", "mai", "mai", "he", "chao", "fu", "mian", \
    "mian", "fu", "pao", "qu", "qu", "mou", "fu", "xian|yan", "lai", "qu", \
    "mian", "chi", "feng", "fu", "qu", "mian", "ma", "me|mo", "me|ma|mo", "hui", \
    "mo", "zou", "nun", "fen", "huang", "huang", "jin", "guang", "tian", "tou", \
    "hong", "hua", "kuang", "hong", "shu", "li", "nian", "chi", "hei", "hei", \
    "yi", "qian", "dan", "xi", "tun", "mo", "mo", "jian|qian", "dai", "chu", \
    "yi|you", "dian|zhan", "yi", "xia", "yan", "qu", "mei", "yan", "qing", "yue|ye", \
    "lai|li", "dang|tang", "du", "can", "yan", "jian|yan", "yan", "zhen|dan", "an", "zhen|yan", \
    "dai|zhun", "can", "wa|yi", "mei", "dan|zhan", "yan", "du", "lu", "xian|zhi", "fen", \
    "fu", "fu", "mian|min|meng", "mian|min|meng", "yuan", "cu", "qu", "zhao|chao", "wa", "zhu", \
    "zhi", "meng", "ao", "bie", "tuo", "bi", "yuan", "zhao|chao", "tuo", "ding|zhen", \
    "mi|jiong", "nai", "ding|zhen", "zi", "gu", "gu", "dong", "fen", "tao", "yuan", \
    "pi", "chang", "gao", "cao|qi", "yuan", "tang", "teng", "shu", "shu", "fen", \
    "fei", "wen", "ba|fei", "diao", "tuo", "zhong", "qu", "sheng", "shi", "you", \
    "shi", "ting", "wu", "xi|ju", "jing", "hun", "xi|ju", "yan", "tu", "si", \
    "xi", "xian", "yan", "lei", "bi", "ya|yao", "qiu", "han", "wu|hui", "wu|hui", \
    "hou|ku", "xie", "he|e", "zha", "xiu", "weng", "zha", "nong", "nang", "qi|ji", \
    "zhai", "ji", "ji|zi", "ji", "ji", "ji|qi|zi", "ji", "chi", "chen", "chen", \
    "he", "ya", "yin|yan", "xie", "bao", "ze", "shi|xie", "zi|chai", "chi", "yan", \
    "ju|zha", "tiao", "ling", "ling", "chu", "quan", "shi|xie", "yin|ken", "nie", "jiu", \
    "yao", "chuo", "yun", "yu|wu", "chu", "qi|yi", "ni", "ce|ze|zha", "chuo|zou", "qu", \
    "yun", "yan", "yu|ou", "e", "wo", "yi", "ci|cuo", "zou", "dian", "chu", \
    "jin", "ya|e", "chi", "chen", "he", "yin|yan|ken", "ju|zha", "ling", "bao", "tiao", \
    "chai|zi", "yin|ken", "wu|yu", "chuo", "qu", "wo", "long|mang", "pang", "gong|wo", "long|pang", \
    "yan", "long|mang", "long", "gong", "kan|ke", "da", "ling", "da", "long|mang", "gong", \
    "kan|ke", "gui|jun|qiu", "qiu", "bie", "gui|jun|qiu", "yue", "chui", "he", "jiao", "xie", \
    "yu"};      
  
    int is_utf8_string(const char *utf)  
    {  
        int length = strlen(utf);  
        int check_sub = 0;  
        int i = 0;  
  
        if ( length > HZ2PY_UTF8_CHECK_LENGTH )  //ֻȡǰ���ض����ȵ��ַ�����֤����  
        {  
            length = HZ2PY_UTF8_CHECK_LENGTH;  
        }  
  
        for ( ; i < length; i ++ )  
        {  
            if ( check_sub == 0 )  
            {  
                if ( (utf[i] >> 7) == 0 )         //0xxx xxxx  
                {  
                    continue;  
                }  
                else if ( (utf[i] & 0xE0) == 0xC0 ) //110x xxxx  
                {  
                    check_sub = 1;  
                }  
                else if ( (utf[i] & 0xF0) == 0xE0 ) //1110 xxxx  
                {  
                    check_sub = 2;  
                }  
                else if ( (utf[i] & 0xF8) == 0xF0 ) //1111 0xxx  
                {  
                    check_sub = 3;  
                }  
                else if ( (utf[i] & 0xFC) == 0xF8 ) //1111 10xx  
                {  
                    check_sub = 4;  
                }  
                else if ( (utf[i] & 0xFE) == 0xFC ) //1111 110x  
                {  
                    check_sub = 5;  
                }  
                else  
                {  
                    return 0;  
                }  
            }  
            else  
            {  
                if ( (utf[i] & 0xC0) != 0x80 )  
                {  
                    return 0;  
                }  
                check_sub --;  
            }  
        }  
        return 1;  
    }  

    void pinyin_utf8(const char* inbuf,char* outbuf, bool initial/*=false*/,bool polyphone_support/*=false*/,bool m_blnFirstBig/*=false*/,bool m_blnAllBiG/*=false*/,bool m_LetterEnd/*=false*/, bool m_unknowSkip/*=true*/,bool m_filterPunc/*=true*/)  
    {  
        int inbuf_len=strlen(inbuf);  
        char *_tmp;  
        char *_tmp2;  
        char py_tmp[HZ2PY_MAX_PINYIN_SIZE] = "";    //ԭʼƴ��  
        char py_tmp2[HZ2PY_MAX_PINYIN_SIZE] = "";   //������ƴ��  
        int uni;  
        int iOutbuf = 0;  
        char sep='\'';          //�ָ���  

        for (int i=0;i<inbuf_len;++i)  
        {  
            if ( (unsigned char)inbuf[i] < 0x80 )  {  //����Ӣ�Ļ�����  
                if(m_filterPunc&&!(inbuf[i]>='a'&&inbuf[i]<='z'||inbuf[i]>='A'&&inbuf[i]<='Z')){  
                    continue;  
                }  
                if(!safeAddToOutbuf(outbuf,iOutbuf,&inbuf[i],1)) return;  
                //if(m_LetterEnd) if(!safeAddToOutbuf(outbuf,iOutbuf,&sep,1)) return;  //Ӣ����ĸҲ��ָ���  
                continue;    
            }else if((inbuf[i]&0xE0)==0xC0){    //�����ַ�����ʾ�Ƿ�ŷ��ϵ��˹������  
                if(i+1>=inbuf_len){  //���һ��������  
                    return;  
                }  
                if(!m_unknowSkip){  
                    if(!safeAddToOutbuf(outbuf,iOutbuf,&inbuf[i],2)) return;  
                }  
                i++;  
            }else if ((inbuf[i] & 0xF0) == 0xE0){   //�����ַ������֡�����������ַ�  
                if(i+2>=inbuf_len){    
                    return;  
                }  
                uni = (((int)(inbuf[i] & 0x0F)) << 12)  
                    | (((int)(inbuf[i+1] & 0x3F)) << 6)  
                    | (inbuf[i+2] & 0x3F);  
                if (uni>19967&&uni<40870)  
                {  
                    memset(py_tmp, '\0', sizeof(char)*HZ2PY_MAX_PINYIN_SIZE);  
                    memset(py_tmp2, '\0', sizeof(char)*HZ2PY_MAX_PINYIN_SIZE);  
  
                    //strcpy_s(py_tmp,HZ2PY_MAX_PINYIN_SIZE, _pinyin_table_[uni - 19968]);  
                    strcpy(py_tmp, _pinyin_table_[uni - 19968]);  
  
                    _tmp = py_tmp;  
                    _tmp2 = py_tmp2;  
  
                    if (initial)   //ֻ֧������ĸ  
                    {  
                        *_tmp2 = *_tmp;  
                        _tmp ++;  
                        _tmp2 ++;  
                        while(*_tmp != '\0')       
                        {  
                            if (*_tmp == '|' || *(_tmp - 1) == '|')   
                            {  
                                *_tmp2 = *_tmp;   
                                _tmp2 ++;  
                            }  
                            _tmp ++;  
                        }  
                        _tmp2 = py_tmp2;  
                    }  
                    else  
                    {  
                        //strcpy_s(py_tmp2,HZ2PY_MAX_PINYIN_SIZE, py_tmp);  
                        strcpy(py_tmp2, py_tmp);
                    }  
  
                    if (m_blnAllBiG)    //ȫ����д  
                    {  
                        while (*_tmp2!='\0')  
                        {  
                            if (*_tmp2>='a'&&*_tmp2<='z')  
                            {  
                                *_tmp2=*_tmp2-32;  
                            }  
                            _tmp2++;  
                        }  
                        _tmp2 = py_tmp2;  
                    }  
  
                    if(m_blnFirstBig){  //����ĸ��д  
                        if (*_tmp2>='a'&&*_tmp2<='z')  
                        {  
                            *_tmp2=*_tmp2-32;  
                        }  
                    }  
  
                    if (!polyphone_support) //��֧�ֶ�����  
                    {  
                        while(*_tmp2 != '\0')   
                        {  
                            if (*_tmp2 == '|')      
                            {  
                                *_tmp2 = '\0';   
                                break;  
                            }  
                            _tmp2 ++;  
                        }  
  
                        _tmp2 = py_tmp2;  
                    }  
  
                    if(!safeAddToOutbuf(outbuf,iOutbuf,py_tmp2,strlen(py_tmp2)))    return;  
                    if(m_LetterEnd) if(!safeAddToOutbuf(outbuf,iOutbuf,&sep,1)) return;    
                    i=i+2;  
                }else if( !m_unknowSkip){  
                    if(!safeAddToOutbuf(outbuf,iOutbuf,&inbuf[i],3)) return;  
                    i=i+2;  
                }  
            }else  if ( (inbuf[i] & 0xF8) == 0xF0 ){//�ĸ��ֽ�  
                if(i+3>=inbuf_len){    
                    return;  
                }  
                if( !m_unknowSkip){  
                    if(!safeAddToOutbuf(outbuf,iOutbuf,&inbuf[i],4)) return;  
                }  
                i=i+3;  
            }else if ( (inbuf[i] & 0xFC) == 0xF8 ){ //����ֽ�  
                if(i+4>=inbuf_len){    
                    return;  
                }  
                if( !m_unknowSkip){  
                    if(!safeAddToOutbuf(outbuf,iOutbuf,&inbuf[i],5)) return;  
                }  
                i=i+4;  
            }else if ( (inbuf[i] & 0xFE) == 0xFC ){ //�����ֽ�  
                if(i+5>=inbuf_len){    
                    return;  
                }  
                if( !m_unknowSkip){  
                    if(!safeAddToOutbuf(outbuf,iOutbuf,&inbuf[i],6)) return;  
                }  
                i=i+5;  
            }else{  
                if ( !m_unknowSkip)  
                {  
                    if(!safeAddToOutbuf(outbuf,iOutbuf,&inbuf[i],1)) return;  
                }  
                i++;  
                //break;  
            }  
        }  
    }  

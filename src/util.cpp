// util.cpp
//
// Copyright (c) 2019 Kristofer Berggren
// All rights reserved.
//
// nmail is distributed under the MIT license, see LICENSE for details.

#include "util.h"

#include <algorithm>
#include <map>
#include <set>

#include <libgen.h>
#include <termios.h>

#include <libetpan/libetpan.h>
#include <ncursesw/ncurses.h>
#include <openssl/sha.h>

#include "apathy/path.hpp"

#include "loghelp.h"
#include "serialized.h"

std::string Util::m_HtmlConvertCmd;
std::string Util::m_ApplicationDir;

std::string Util::SHA256(const std::string &p_Str)
{
  unsigned char hash[SHA256_DIGEST_LENGTH];
  SHA256_CTX sha256;
  SHA256_Init(&sha256);
  SHA256_Update(&sha256, p_Str.c_str(), p_Str.size());
  SHA256_Final(hash, &sha256);
  return Serialized::ToHex(std::string((char*)hash, SHA256_DIGEST_LENGTH));
}

bool Util::Exists(const std::string &p_Path)
{
  struct stat sb;
  return (stat(p_Path.c_str(), &sb) == 0);
}

std::string Util::ReadFile(const std::string &p_Path)
{
  std::ifstream file(p_Path);
  std::stringstream ss;
  ss << file.rdbuf();
  return ss.str();
}

void Util::WriteFile(const std::string &p_Path, const std::string &p_Str)
{
  MkDir(DirName(p_Path));
  std::ofstream file(p_Path);
  file << p_Str;
}

std::string Util::BaseName(const std::string &p_Path)
{
  char* path = strdup(p_Path.c_str());
  char* bname = basename(path);
  std::string rv(bname);
  free(path);
  return rv;
}

std::string Util::DirName(const std::string &p_Path)
{
  char *buf = strdup(p_Path.c_str());
  std::string rv = std::string(dirname(buf));
  free(buf);
  return rv;
}

void Util::MkDir(const std::string &p_Path)
{
  apathy::Path::makedirs(p_Path);
}

void Util::RmDir(const std::string &p_Path)
{
  apathy::Path::rmdirs(apathy::Path(p_Path));
}

void Util::Touch(const std::string &p_Path)
{
  utimensat(0, p_Path.c_str(), NULL, 0);
}

std::string Util::GetApplicationDir()
{
  return m_ApplicationDir;
}

void Util::SetApplicationDir(const std::string &p_Path)
{
  m_ApplicationDir = p_Path + "/";
}

std::string Util::GetTempDir()
{
  return GetApplicationDir() + std::string("temp/");
}

void Util::InitTempDir()
{
  Util::RmDir(GetTempDir());
  Util::MkDir(GetTempDir());
}

void Util::CleanupTempDir()
{
  Util::RmDir(GetTempDir());
}

std::string Util::GetTempFilename(const std::string &p_Suffix)
{
  std::string name = GetTempDir() + std::string("temp.XX" "XX" "XX") + p_Suffix;
  char* cname = strdup(name.c_str());
  int fd = mkstemps(cname, p_Suffix.length());
  if (fd != -1)
  {
    close(fd);
  }

  name = std::string(cname);
  free(cname);
  return name;
}

void Util::DeleteFile(const std::string &p_Path)
{
  unlink(p_Path.c_str());
}

time_t Util::MailtimeToTimet(mailimf_date_time *p_Dt)
{
  char buf[128];
  snprintf(buf, sizeof(buf), "%04i-%02i-%02i %02i:%02i:%02i",
           p_Dt->dt_year, p_Dt->dt_month, p_Dt->dt_day, p_Dt->dt_hour,
           p_Dt->dt_min, p_Dt->dt_sec);
  int offs = p_Dt->dt_zone;

  struct tm tm;
  memset(&tm, 0, sizeof(struct tm));
  strptime(buf, "%Y-%m-%d %H:%M:%S", &tm);
  tm.tm_isdst = -1;

  time_t t = timegm(&tm);
  int offs_h = offs / 100;
  int offs_m = offs % 100;
  t -= offs_h * 3600;
  t -= offs_m * 60;

  return t;
}

std::string Util::GetHtmlConvertCmd()
{
  return m_HtmlConvertCmd;
}

void Util::SetHtmlConvertCmd(const std::string &p_HtmlConvertCmd)
{
  m_HtmlConvertCmd = p_HtmlConvertCmd;
}

std::string Util::GetDefaultHtmlConvertCmd()
{
  std::string result;
  const std::string& commandOutPath = Util::GetTempFilename(".txt");
  const std::string& command =
    std::string("which lynx elinks links 2> /dev/null | head -1 > ") + commandOutPath;
  if (system(command.c_str()) == 0)
  {
    std::string output = Util::ReadFile(commandOutPath);
    output.erase(std::remove(output.begin(), output.end(), '\n'), output.end());
    if (!output.empty())
    {
      result = output + " -dump";
    }
  }

  Util::DeleteFile(commandOutPath);

  return result;
}

void Util::ReplaceString(std::string &p_Str, const std::string &p_Search,
                         const std::string &p_Replace)
{
  size_t pos = 0;
  while ((pos = p_Str.find(p_Search, pos)) != std::string::npos)
  {
    p_Str.replace(pos, p_Search.length(), p_Replace);
    pos += p_Replace.length();
  }
}

std::string Util::ReduceIndent(const std::string &p_Str, int p_Cnt)
{
  std::string tmpstr = "\n" + p_Str;
  std::string findstr = "\n ";
  std::string replacestr = "\n";
  for (int i = 0; i < p_Cnt; ++i)
  {
    ReplaceString(tmpstr, findstr, replacestr);
  }

  return tmpstr.substr(1);
}

std::string Util::AddIndent(const std::string &p_Str, const std::string &p_Indent)
{
  std::string tmpstr = "\n" + p_Str;
  std::string findstr = "\n";
  std::string replacestr = "\n" + p_Indent;
  ReplaceString(tmpstr, findstr, replacestr);

  return tmpstr.substr(1);
}

std::string Util::MakeReplySubject(const std::string &p_Str)
{
  std::set<std::string> replyPrefixes = { "re:", "sv:" };
  std::string oldPrefix = ToLower(p_Str.substr(0, 3));
  if (replyPrefixes.find(oldPrefix) == replyPrefixes.end())
  {
    return "Re: " + p_Str;
  }
  return p_Str;
}

std::string Util::MakeForwardSubject(const std::string &p_Str)
{
  std::set<std::string> replyPrefixes = { "fw", "fwd", "vb" };
  std::vector<std::string> oldSubjectSplit= Split(p_Str, ':');
  bool hasFwdPrefix = false;
  if (oldSubjectSplit.size() > 1)
  {
    std::string oldPrefix = ToLower(oldSubjectSplit.at(0));
    if (replyPrefixes.find(oldPrefix) != replyPrefixes.end())
    {
      hasFwdPrefix = true;
    }
  }

  return hasFwdPrefix ? p_Str : ("Fwd: " + p_Str);
}

std::string Util::GetHostname()
{
  char hostname[256]; // @todo: use HOST_NAME_MAX?
  gethostname(hostname, sizeof(hostname));
  return std::string(hostname);
}

std::string Util::ToString(const std::wstring& p_WStr)
{
  size_t len = std::wcstombs(nullptr, p_WStr.c_str(), 0);
  std::vector<char> cstr(len + 1);
  std::wcstombs(&cstr[0], p_WStr.c_str(), len);
  std::string str(&cstr[0], len);
  return str;
}

std::wstring Util::ToWString(const std::string& p_Str)
{
  size_t len = mbstowcs(nullptr, p_Str.c_str(), 0);
  std::vector<wchar_t> wcstr(len + 1);
  std::mbstowcs(&wcstr[0], p_Str.c_str(), len);
  std::wstring wstr(&wcstr[0], len);
  return wstr;
}

std::string Util::TrimPadString(const std::string &p_Str, size_t p_Len)
{
  std::string str = p_Str;
  if (str.size() > p_Len)
  {
    str = str.substr(0, p_Len);
  }
  else if (str.size() < p_Len)
  {
    str = str + std::string(p_Len - str.size(), ' ');
  }
  return str;
}

std::wstring Util::TrimPadWString(const std::wstring &p_Str, size_t p_Len)
{
  std::wstring str = p_Str;
  if (str.size() > p_Len)
  {
    str = str.substr(0, p_Len);
  }
  else if (str.size() < p_Len)
  {
    str = str + std::wstring(p_Len - str.size(), ' ');
  }
  return str;
}

std::string Util::ToLower(const std::string &p_Str)
{
  std::string lower = p_Str;
  std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
  return lower;
}

std::vector<std::string> Util::Split(const std::string &p_Str, char p_Sep)
{
  std::vector<std::string> vec;
  if (!p_Str.empty())
  {
    std::stringstream ss(p_Str);
    while (ss.good())
    {
      std::string str;
      getline(ss, str, p_Sep);
      vec.push_back(str);
    }
  }
  return vec;
}

std::string Util::Trim(const std::string &p_Str)
{
  std::string space = " ";
  const auto strBegin = p_Str.find_first_not_of(space);
  if (strBegin == std::string::npos) return "";

  const auto strEnd = p_Str.find_last_not_of(space);
  const auto strRange = strEnd - strBegin + 1;

  return p_Str.substr(strBegin, strRange);
}

std::string Util::GetConfigDir()
{
  return std::string(getenv("HOME")) + std::string("/.nchat");
}

int Util::GetKeyCode(const std::string& p_KeyName)
{
  static std::map<std::string, int> keyCodes =
    {
      // additional keys
      { "KEY_TAB", KEY_TAB},
      { "KEY_RETURN", KEY_RETURN},
      { "KEY_SPACE", KEY_SPACE},

      // ctrl keys
      { "KEY_CTRL@", 0},
      { "KEY_CTRLA", 1},
      { "KEY_CTRLB", 2},
      { "KEY_CTRLC", 3},
      { "KEY_CTRLD", 4},
      { "KEY_CTRLE", 5},
      { "KEY_CTRLF", 6},
      { "KEY_CTRLG", 7},
      { "KEY_CTRLH", 8},
      { "KEY_CTRLI", 9},
      { "KEY_CTRLJ", 10},
      { "KEY_CTRLK", 11},
      { "KEY_CTRLL", 12},
      { "KEY_CTRLM", 13},
      { "KEY_CTRLN", 14},
      { "KEY_CTRLO", 15},
      { "KEY_CTRLP", 16},
      { "KEY_CTRLQ", 17},
      { "KEY_CTRLR", 18},
      { "KEY_CTRLS", 19},
      { "KEY_CTRLT", 20},
      { "KEY_CTRLU", 21},
      { "KEY_CTRLV", 22},
      { "KEY_CTRLW", 23},
      { "KEY_CTRLX", 24},
      { "KEY_CTRLY", 25},
      { "KEY_CTRLZ", 26},
      { "KEY_CTRL[", 27},
      { "KEY_CTRL\\", 28},
      { "KEY_CTRL]", 29},
      { "KEY_CTRL^", 30},
      { "KEY_CTRL_", 31},

      // ncurses keys
      { "KEY_DOWN", KEY_DOWN },
      { "KEY_UP", KEY_UP },
      { "KEY_LEFT", KEY_LEFT },
      { "KEY_RIGHT", KEY_RIGHT },
      { "KEY_HOME", KEY_HOME },
      { "KEY_BACKSPACE", KEY_SYS_BACKSPACE },
      { "KEY_F0", KEY_F0 },
      { "KEY_F1", KEY_F(1) },
      { "KEY_F2", KEY_F(2) },
      { "KEY_F3", KEY_F(3) },
      { "KEY_F4", KEY_F(4) },
      { "KEY_F5", KEY_F(5) },
      { "KEY_F6", KEY_F(6) },
      { "KEY_F7", KEY_F(7) },
      { "KEY_F8", KEY_F(8) },
      { "KEY_F9", KEY_F(9) },
      { "KEY_F10", KEY_F(10) },
      { "KEY_F11", KEY_F(11) },
      { "KEY_F12", KEY_F(12) },
      { "KEY_DL", KEY_DL },
      { "KEY_IL", KEY_IL },
      { "KEY_DC", KEY_DC },
      { "KEY_IC", KEY_IC },
      { "KEY_EIC", KEY_EIC },
      { "KEY_CLEAR", KEY_CLEAR },
      { "KEY_EOS", KEY_EOS },
      { "KEY_EOL", KEY_EOL },
      { "KEY_SF", KEY_SF },
      { "KEY_SR", KEY_SR },
      { "KEY_NPAGE", KEY_NPAGE },
      { "KEY_PPAGE", KEY_PPAGE },
      { "KEY_STAB", KEY_STAB },
      { "KEY_CTAB", KEY_CTAB },
      { "KEY_CATAB", KEY_CATAB },
      { "KEY_ENTER", KEY_ENTER },
      { "KEY_PRINT", KEY_PRINT },
      { "KEY_LL", KEY_LL },
      { "KEY_A1", KEY_A1 },
      { "KEY_A3", KEY_A3 },
      { "KEY_B2", KEY_B2 },
      { "KEY_C1", KEY_C1 },
      { "KEY_C3", KEY_C3 },
      { "KEY_BTAB", KEY_BTAB },
      { "KEY_BEG", KEY_BEG },
      { "KEY_CANCEL", KEY_CANCEL },
      { "KEY_CLOSE", KEY_CLOSE },
      { "KEY_COMMAND", KEY_COMMAND },
      { "KEY_COPY", KEY_COPY },
      { "KEY_CREATE", KEY_CREATE },
      { "KEY_END", KEY_END },
      { "KEY_EXIT", KEY_EXIT },
      { "KEY_FIND", KEY_FIND },
      { "KEY_HELP", KEY_HELP },
      { "KEY_MARK", KEY_MARK },
      { "KEY_MESSAGE", KEY_MESSAGE },
      { "KEY_MOVE", KEY_MOVE },
      { "KEY_NEXT", KEY_NEXT },
      { "KEY_OPEN", KEY_OPEN },
      { "KEY_OPTIONS", KEY_OPTIONS },
      { "KEY_PREVIOUS", KEY_PREVIOUS },
      { "KEY_REDO", KEY_REDO },
      { "KEY_REFERENCE", KEY_REFERENCE },
      { "KEY_REFRESH", KEY_REFRESH },
      { "KEY_REPLACE", KEY_REPLACE },
      { "KEY_RESTART", KEY_RESTART },
      { "KEY_RESUME", KEY_RESUME },
      { "KEY_SAVE", KEY_SAVE },
      { "KEY_SBEG", KEY_SBEG },
      { "KEY_SCANCEL", KEY_SCANCEL },
      { "KEY_SCOMMAND", KEY_SCOMMAND },
      { "KEY_SCOPY", KEY_SCOPY },
      { "KEY_SCREATE", KEY_SCREATE },
      { "KEY_SDC", KEY_SDC },
      { "KEY_SDL", KEY_SDL },
      { "KEY_SELECT", KEY_SELECT },
      { "KEY_SEND", KEY_SEND },
      { "KEY_SEOL", KEY_SEOL },
      { "KEY_SEXIT", KEY_SEXIT },
      { "KEY_SFIND", KEY_SFIND },
      { "KEY_SHELP", KEY_SHELP },
      { "KEY_SHOME", KEY_SHOME },
      { "KEY_SIC", KEY_SIC },
      { "KEY_SLEFT", KEY_SLEFT },
      { "KEY_SMESSAGE", KEY_SMESSAGE },
      { "KEY_SMOVE", KEY_SMOVE },
      { "KEY_SNEXT", KEY_SNEXT },
      { "KEY_SOPTIONS", KEY_SOPTIONS },
      { "KEY_SPREVIOUS", KEY_SPREVIOUS },
      { "KEY_SPRINT", KEY_SPRINT },
      { "KEY_SREDO", KEY_SREDO },
      { "KEY_SREPLACE", KEY_SREPLACE },
      { "KEY_SRIGHT", KEY_SRIGHT },
      { "KEY_SRSUME", KEY_SRSUME },
      { "KEY_SSAVE", KEY_SSAVE },
      { "KEY_SSUSPEND", KEY_SSUSPEND },
      { "KEY_SUNDO", KEY_SUNDO },
      { "KEY_SUSPEND", KEY_SUSPEND },
      { "KEY_UNDO", KEY_UNDO },
      { "KEY_MOUSE", KEY_MOUSE },
      { "KEY_RESIZE", KEY_RESIZE },
      { "KEY_EVENT", KEY_EVENT },
    };

  int keyCode = -1;
  std::map<std::string, int>::iterator it = keyCodes.find(p_KeyName);
  if (it != keyCodes.end())
  {
    keyCode = it->second;
  }
  else if ((p_KeyName.size() > 2) && (p_KeyName.substr(0, 2) == "0x"))
  {
    keyCode = strtol(p_KeyName.c_str(), 0, 16);
  }
  else if (p_KeyName.size() == 1)
  {
    keyCode = (int)p_KeyName.at(0);
  }
  else
  {
    LOG_WARNING("warning: unknown key \"%s\"", p_KeyName.c_str());
  }

  return keyCode;
}

std::vector<std::wstring> Util::WordWrap(std::wstring p_Text, unsigned p_LineLength)
{
  int pos = 0;
  int wrapLine = 0;
  int wrapPos = 0;
  return WordWrap(p_Text, p_LineLength, pos, wrapLine, wrapPos);
}

std::vector<std::wstring> Util::WordWrap(std::wstring p_Text, unsigned p_LineLength, int p_Pos,
                                         int &p_WrapLine, int &p_WrapPos)
{
  std::wostringstream wrapped;
  std::vector<std::wstring> lines;

  p_WrapLine = 0;
  p_WrapPos = 0;

  {
    std::wstring line;
    std::wistringstream textss(p_Text);
    while (std::getline(textss, line))
    {
      std::wstring linePart = line;
      while (true)
      {
        if (linePart.size() >= p_LineLength)
        {
          size_t breakAt = linePart.rfind(L' ', p_LineLength);
          if (breakAt == std::wstring::npos)
          {
            breakAt = p_LineLength;
          }
          
          lines.push_back(linePart.substr(0, breakAt));
          linePart = linePart.substr(breakAt + 1);
        }
        else
        {
          lines.push_back(linePart);
          linePart.clear();
          break;
        }
      }
    }
  }

  for (auto& line : lines)
  {
    if (p_Pos > 0)
    {
      int lineLength = line.size() + 1;
      if (lineLength <= p_Pos)
      {
        p_Pos -= lineLength;
        ++p_WrapLine;
      }
      else
      {
        p_WrapPos = p_Pos;
        p_Pos = 0;
      }
    }
  }

  return lines;
}

std::vector<std::string> Util::WordWrap(std::string p_Text, unsigned p_LineLength)
{
  int pos = 0;
  int wrapLine = 0;
  int wrapPos = 0;
  return WordWrap(p_Text, p_LineLength, pos, wrapLine, wrapPos);
}

std::vector<std::string> Util::WordWrap(std::string p_Text, unsigned p_LineLength, int p_Pos,
                                        int &p_WrapLine, int &p_WrapPos)
{
  std::ostringstream wrapped;
  std::vector<std::string> lines;

  p_WrapLine = 0;
  p_WrapPos = 0;

  {
    std::string line;
    std::istringstream textss(p_Text);
    while (std::getline(textss, line))
    {
      std::string linePart = line;
      while (true)
      {
        if (linePart.size() >= p_LineLength)
        {
          size_t breakAt = linePart.rfind(' ', p_LineLength);
          if (breakAt == std::string::npos)
          {
            breakAt = p_LineLength;
          }

          lines.push_back(linePart.substr(0, breakAt));
          linePart = linePart.substr(breakAt + 1);
        }
        else
        {
          lines.push_back(linePart);
          linePart.clear();
          break;
        }
      }
    }
  }

  for (auto& line : lines)
  {
    if (p_Pos > 0)
    {
      int lineLength = line.size() + 1;
      if (lineLength <= p_Pos)
      {
        p_Pos -= lineLength;
        ++p_WrapLine;
      }
      else
      {
        p_WrapPos = p_Pos;
        p_Pos = 0;
      }
    }
  }

  return lines;
}

std::string Util::GetPass()
{
  std::string pass;
  struct termios told, tnew;

  if (tcgetattr(STDIN_FILENO, &told) == 0)
  {
    memcpy(&tnew, &told, sizeof(struct termios));
    tnew.c_lflag &= ~ECHO;
    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &tnew) == 0)
    {
      std::getline(std::cin, pass);
      tcsetattr(STDIN_FILENO, TCSAFLUSH, &told);
      std::cout << std::endl;
    }
  }

  return pass;
}

std::wstring Util::Join(const std::vector<std::wstring>& p_Lines)
{
  std::wstring str;
  bool first = true;
  for (auto& line : p_Lines)
  {
    if (!first)
    {
      str += L"\n";
    }
    else
    {
      first = false;
    }
      
    str += line;
  }
  return str;
}

std::string Util::Join(const std::vector<std::string>& p_Lines, const std::string& p_Delim)
{
  std::string str;
  bool first = true;
  for (auto& line : p_Lines)
  {
    if (!first)
    {
      str += p_Delim;
    }
    else
    {
      first = false;
    }
      
    str += line;
  }
  return str;
}
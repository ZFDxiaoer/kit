#ifndef CMD_PARSER_MANAGER_H
#define CMD_PARSER_MANAGER_H

#include "util_singleton.h"
#include "cmd.pb.h"

class CmdParserManager : public CSingleton<CmdParserManager>
{
    public:
        uint32_t dealProtocal(uint32_t, const string&, const pb::CmdData& stReq, pb::CmdData& stRsp);
};

#define CMD_PARSER_MANAGER CmdParserManager::GetInstance()

#endif
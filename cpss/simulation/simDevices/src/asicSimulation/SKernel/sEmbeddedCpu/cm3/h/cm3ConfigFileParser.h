#ifndef _CM3_CONFIG_FILE_PARSER_H
#define _CM3_CONFIG_FILE_PARSER_H


GT_STATUS cm3lnConfigFileParserParse
(
   const char *file,
   size_t size
);

GT_STATUS cm3GenSwitchRegisterSet
(
    GT_U32 address,
    GT_U32 data,
    GT_U32 mask
);

GT_STATUS cm3GenSwitchRegisterGet
(
    GT_U32 address,
    GT_U32 *data,
    GT_U32 mask
);

GT_STATUS cm3ParseCommand
(
    char *command
);



#endif /* _CM3_CONFIG_FILE_PARSER_H */


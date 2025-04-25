#pragma once

#include <string>
#include <vector>
#include <memory>

typedef struct yy_buffer_state * YY_BUFFER_STATE;
extern int yyparse();
extern int yylineno;
extern YY_BUFFER_STATE yy_scan_string(const char * str);
extern YY_BUFFER_STATE yy_switch_to_buffer(YY_BUFFER_STATE buffer);
extern void yy_delete_buffer(YY_BUFFER_STATE buffer);

namespace toyc::parser {

int parseFile(const std::string &fileName);
int parseContent(const std::string &content);

} // namespace ddlbx::parser
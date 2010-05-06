REM it is necessary in order to get a correct line code
pushd ..

echo Bison
bison -t -d -v -p ai -o ai_service/script_parser_yacc.cpp ai_service/script_parser.yacc
del ai_service\script_parser_yacc.h
rename ai_service\script_parser_yacc.hpp script_parser_yacc.h

Echo Flex
flex -f -8 -Pcf -oai_service/script_parser_lex.cpp ai_service/script_parser.lex
cat ai_service\script_parser_lex.cpp | sed -e "s/#include <unistd.h>/#ifdef WIN32\n#include <io.h>\n#else \/\/ WIN32\n#include <unistd.h>\n#endif \/\/ WIN32\n/g" > ai_service\toto.cpp
del ai_service\script_parser_lex.cpp
rename ai_service\toto.cpp script_parser_lex.cpp

popd

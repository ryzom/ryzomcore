# Ordered area definitions shared by both merge reports: (key, title, path prefixes).
# The first matching prefix wins, so more specific entries come first.
AREAS = [
    ('build', 'Build system, CI and repository files', [
        'CMakeLists.txt', 'CMakeModules/', 'CMakePresets.json', 'cmake/', 'config.h.cmake', 'vcpkg', 'tool/',
        '.github/', '.gitlab-ci.yml', 'azure-pipelines.yml', '.devcontainer/', 'docker', 'chocolatey.config',
        '.gitignore', '.gitattributes', '.clang-format', '.clang-tidy', '.editorconfig', '.hgeol', '.hgignore',
        '.hgtags', '.travis.yml', 'nelDashBuild', 'changelog.template', 'README', 'dist/', 'macosx-build/', 'win-build/',
        'logs/', 'docs/', 'CONTRIBUTING', 'LICENSE', 'COPYING', 'CODE_OF_CONDUCT', 'SECURITY']),
    ('nel-include', 'NeL headers (nel/include)', ['nel/include/']),
    ('nel-src', 'NeL libraries (nel/src)', ['nel/src/']),
    ('nel-tools', 'NeL tools and samples', ['nel/tools/', 'nel/samples/', 'nel/3rdparty/', 'nel/']),
    ('nelns', 'NeL network services (nelns)', ['nelns/']),
    ('game-share', 'Ryzom shared code and data (ryzom/common)', ['ryzom/common/']),
    ('client-src', 'Ryzom client sources', ['ryzom/client/src/']),
    ('client-data', 'Ryzom client data: UI XML, Lua, configuration', ['ryzom/client/']),
    ('server-egs', 'Entities game service (EGS)', ['ryzom/server/src/entities_game_service/']),
    ('server-ais', 'AI service', ['ryzom/server/src/ai_service/', 'ryzom/server/src/ai_share/', 'ryzom/server/src/ai_data_service/']),
    ('server-other', 'Other services and server shared code', ['ryzom/server/src/']),
    ('server-tools', 'Server tools and scripts (ryzom/server/tools)', ['ryzom/server/tools/']),
    ('server-www', 'Server web pages (ryzom/server/www)', ['ryzom/server/www/']),
    ('server-rest', 'Server tree, other files', ['ryzom/server/']),
    ('ryzom-tools', 'Ryzom tools (ryzom/tools)', ['ryzom/tools/']),
    ('ryzom-rest', 'Ryzom tree, other files', ['ryzom/']),
    ('web', 'Web (web/)', ['web/']),
    ('studio', 'Studio, Snowballs and personal projects', ['studio/', 'snowballs2/', 'personal/']),
]

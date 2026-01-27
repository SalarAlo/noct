#include "noct/module/Module.h"

namespace Noct {

Module::Module(std::string_view name, bool automaticImport)
    : m_Name(name)
    , m_AutoImport(automaticImport) {
}

}

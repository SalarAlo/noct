#pragma once

#include <string>

#include "noct/Environment.h"

namespace Noct {
class Module {
public:
	Module(std::string_view name, bool automaticImport);
	void AddSymbol(const EnvironmentVariable& var);

private:
	std::string m_Name;
	bool m_AutoImport;
	std::shared_ptr<Environment> m_GlobalEnv;
	size_t m_SlotCount;
};

}

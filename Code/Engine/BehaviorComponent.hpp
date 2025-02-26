#pragma once

#include "EngineCommon.hpp"
#include "Common/Types.hpp"
#include "Common/Filesystem.hpp"

namespace x {
    class BehaviorComponent {
    public:
        BehaviorComponent() = default;

        void LoadFromFile(const str& filename) {
            using namespace Filesystem;
            auto scriptSource = FileReader::ReadAllText(Path(filename));

            if (!scriptSource.empty()) {
                mScriptSource = std::move(scriptSource);
                mScriptPath   = filename;
                mScriptId     = Path(filename).Str();
            }
        }

        void Reload() {
            if (!mScriptPath.empty()) { LoadFromFile(mScriptPath); }
        }

        void UpdateSource(const str& source) {
            using namespace Filesystem;
            mScriptSource = source;
            FileWriter::WriteAllText(Path(mScriptPath), source);
        }

        X_NODISCARD const str& GetSource() const {
            return mScriptSource;
        }

        X_NODISCARD const str& GetId() const {
            return mScriptId;
        }

    private:
        str mScriptSource;
        str mScriptPath;
        str mScriptId;
    };
}  // namespace x
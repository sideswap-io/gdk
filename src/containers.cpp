#include "containers.hpp"
#include "amount.hpp"
#include "assertion.hpp"
#include "ga_wally.hpp"

namespace ga {
namespace sdk {

    bool json_rename_key(nlohmann::json& data, const std::string& from_key, const std::string& to_key)
    {
        auto p = data.find(from_key);
        if (p == data.end()) {
            return false;
        }
        data[to_key] = *p;
        data.erase(p);
        return true;
    }

    amount json_get_amount(const nlohmann::json& data, const std::string& key)
    {
        return amount(data.at(key).get<amount::value_type>());
    }

    amount json_get_amount(const nlohmann::json& data, const std::string& key, const amount& default_value)
    {
        return amount(json_get_value(data, key, default_value.value()));
    }

} // namespace sdk
} // namespace ga

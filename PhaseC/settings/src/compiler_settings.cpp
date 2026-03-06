#include "settings/compiler_settings.hpp"
#include <algorithm>
#include <set>
#include <vector>

#include "driver/exception.hpp"
#include "arguinator/arguinator.hpp"
#include "support/debug_tools.hpp"

namespace alpha::settings
{
struct ConfigDataNames
{
    #define REGISTER_SETTING(REQUIRED, ARITY, NAME, VALUE_TYPE, HELP_MSG) \
        static constexpr char NAME[] = #NAME;
    CONFIG_DATA_SETTINGS(REGISTER_SETTING)
    #undef  REGISTER_SETTING
};
} // namespace alpha::settings

namespace alpha
{
#define MAKE_SETTING(REQUIRED, ARITY, NAME, HELP_MSG) \
    Setting{\
        .required = REQUIRED,\
        .arity = ARITY,\
        .name = #NAME,\
        .help = HELP_MSG\
    },

#define MAKE_SETTING_WITH_VALUE_TYPE(REQUIRED, ARITY, NAME, VALUE_TYPE, HELP_MSG) \
    MAKE_SETTING(REQUIRED, ARITY,NAME, HELP_MSG)

SettingManager::SettingManager()
    : all_settings_(
        std::vector{
            JOB_SETTINGS(MAKE_SETTING)
            CONFIG_FLAG_SETTINGS(MAKE_SETTING)
            CONFIG_DATA_SETTINGS(MAKE_SETTING_WITH_VALUE_TYPE)
            EXPR_OPT_SETTINGS(MAKE_SETTING)
            IR_OPT_SETTINGS(MAKE_SETTING)
        }
    ) {}

#undef  MAKE_SETTING
#undef  MAKE_SETTING_WITH_VALUE_TYPE

void
SettingManager::parse_settings(const arguinator::Parser& arg_parser)
{
    config_flag_settings_.set(SettingManager::parse_config_flag_settings(arg_parser));
    config_data_settings_.set(SettingManager::parse_config_data_settings(arg_parser));
    expr_opt_settings_.set(SettingManager::parse_expr_opt_settings(arg_parser));
    ir_opt_settings_.set(SettingManager::parse_ir_opt_settings(arg_parser));
}

const settings::ExprOpts&
SettingManager::expr_opt_settings() const
{
    if (!expr_opt_settings_.is_assigned())
        throw std::logic_error(ATTACH_CONTEXT("Tried to access unassigned expr_opt_settings_"));
    return expr_opt_settings_.get();
}

const settings::IROpts&
SettingManager::ir_opt_settings() const
{
    if (!ir_opt_settings_.is_assigned())
        throw std::logic_error(ATTACH_CONTEXT("Tried to access unassigned expr_opt_settings_"));
    return ir_opt_settings_.get();
}

const settings::ConfigFlags&
SettingManager::config_flag_settings() const
{
    if (!config_flag_settings_.is_assigned())
        throw std::logic_error(ATTACH_CONTEXT("Tried to access unassigned config_flag_settings_"));
    return config_flag_settings_.get();
}

const settings::ConfigData&
SettingManager::config_data_settings() const
{
    if (!config_data_settings_.is_assigned())
        throw std::logic_error(ATTACH_CONTEXT("Tried to access unassigned config_data_settings_"));
    return config_data_settings_.get();
}

settings::ConfigFlags
SettingManager::parse_config_flag_settings(const arguinator::Parser& arg_parser)
{
    #define QUERY(REQUIRED, ARITY, NAME, HELP_MSG) \
        .NAME = arg_parser[#NAME].is_provided(),
    return {
        CONFIG_FLAG_SETTINGS(QUERY)
    };
    #undef QUERY
}


settings::ConfigData
SettingManager::parse_config_data_settings(const arguinator::Parser& arg_parser)
{
    DEBUG_SMART_ASSERT(
        arg_parser[settings::ConfigDataNames::source].is_provided() &&
        "This field should be required!"
    );

    auto max_error_extractor = [&arg_parser]()
    {
        const auto flag_name = settings::ConfigDataNames::max_errors;

        const auto max_errors = arg_parser[flag_name].is_provided()
                                ? std::stoull(arg_parser[flag_name].get_input())
                                : std::numeric_limits<decltype(settings::ConfigData::max_errors
                                )>::max();

        if (max_errors == 0)
            throw alpha::exception::CLIOptionValueError(flag_name, std::to_string(max_errors));

        return max_errors;
    };

    return {
        .source = arg_parser[settings::ConfigDataNames::source].get_input(),
        .max_errors = max_error_extractor(),
    };
}

settings::ExprOpts
SettingManager::parse_expr_opt_settings(const arguinator::Parser& arg_parser)
{
    #define QUERY(REQUIRED, ARITY, NAME, HELP_MSG) \
    .NAME = arg_parser[#NAME].is_provided(),
    return {
        EXPR_OPT_SETTINGS(QUERY)
    };
    #undef QUERY
}

settings::IROpts
SettingManager::parse_ir_opt_settings(const arguinator::Parser& arg_parser)
{
    #define QUERY(REQUIRED, ARITY, NAME, HELP_MSG) \
    .NAME = arg_parser[#NAME].is_provided(),
    return {
        IR_OPT_SETTINGS(QUERY)
    };
    #undef QUERY
}
} // namespace alpha

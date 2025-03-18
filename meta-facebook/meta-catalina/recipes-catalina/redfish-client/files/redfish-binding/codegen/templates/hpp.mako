// @generated
#pragma once
% if len(cpp_def.include_generated_headers) > 0:

% for header in sorted(cpp_def.include_generated_headers):
#include "${header}.hpp"
% endfor
% endif
% if isinstance(cpp_def, cpp.CppObjectDef) or isinstance(cpp_def, cpp.CppEnumDef):

#include "common.hpp"
% endif
% if len(cpp_def.include_headers) > 0:

% for header in sorted(cpp_def.include_headers):
#include <${header}>
% endfor
% endif
% if isinstance(cpp_def, cpp.CppEnumDef):

#include <string>
#include <unordered_map>
% endif

namespace redfishlib
{
% if len(forward_declarations) > 0:

% for namespace, class_names in sorted(forward_declarations.items()):
namespace ${namespace}
{
% for class_name in sorted(class_names):
class ${class_name};
% endfor
} // namespace ${namespace}
% endfor
% endif

namespace ${cpp_def.identifier.namespace}
{

% if isinstance(cpp_def, cpp.CppTypeAliasDef):
using ${cpp_def.identifier.id} = ${cpp_def.type_alias.get_name(cpp_def.identifier.namespace)};
% elif isinstance(cpp_def, cpp.CppObjectDef):
class ${cpp_def.identifier.id} : public ResourceBaseWithError
{
  public:
    % for property in cpp_def.properties:
    Property<${property.cpp_type.get_name(cpp_def.identifier.namespace)}>& get${property.name_as_member}()
    {
      return ${property.name_as_member}_;
    }

    % endfor
  protected:
    IProperty* findProperty(const std::string& name) override
    {
      % for property in cpp_def.properties:
      if (name == ${property.name_as_member}_.name())
      {
        return &${property.name_as_member}_;
      }
      % endfor
      return ResourceBaseWithError::findProperty(name);
    }
    
  % if len(cpp_def.properties) > 0:
  private:
    % for property in cpp_def.properties:
    Property<${property.cpp_type.get_name(cpp_def.identifier.namespace)}> ${property.name_as_member}_{"${property.name}"};
    % endfor
  % endif
};
% elif isinstance(cpp_def, cpp.CppEnumDef):
class ${cpp_def.identifier.id} : public JsonDeserializable
{
  public:
    enum class Enum
    {
      % for enum_key in cpp_def.enum:
      ${enum_key},
      % endfor
    };

    void fromJson(const nlohmann::json& json) override
    {
      static const std::unordered_map<std::string, Enum> kStrToEnumMap{
        % for enum_key, enum_value in cpp_def.enum.items():
        {"${enum_value}", Enum::${enum_key}},
        % endfor
      };
      if (auto it = kStrToEnumMap.find(json.template get<std::string>()); it != kStrToEnumMap.end())
      {
        value_ = it->second;
      }
    }

    Enum& value()
    {
      return value_;
    }

  private:
    Enum value_;
};
% endif

inline ${cpp_def.identifier.id} parse${cpp_def.identifier.id}(const std::string& json)
{
  return nlohmann::json::parse(json).template get<${cpp_def.identifier.id}>();
}

} // namespace ${cpp_def.identifier.namespace}

} // namespace redfishlib

# ----------------------------------------------------------------------------
# Copyright 2019-2021 Diligent Graphics LLC
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# In no event and under no legal theory, whether in tort (including negligence),
# contract, or otherwise, unless required by applicable law (such as deliberate
# and grossly negligent acts) or agreed to in writing, shall any Contributor be
# liable for any damages, including any direct, indirect, special, incidental,
# or consequential damages of any character arising as a result of this License or
# out of the use or inability to use the software (including but not limited to damages
# for loss of goodwill, work stoppage, computer failure or malfunction, or any and
# all other commercial damages or losses), even if such Contributor has been advised
# of the possibility of such damages.
# ----------------------------------------------------------------------------

from jinja2 import Template

CXX_ENUM_SERIALIZE_TEMPLATE = Template(''' 
{%- macro serialize_enum(type, xitems) -%}
NLOHMANN_JSON_SERIALIZE_ENUM(
{{type}}, {
{%- for item in xitems %}
{ {{ item['value'] }}, "{{ item['name'] }}" },
{%- endfor %}
})
{% endmacro -%}
{%- for type, xitems in enums %}
{{serialize_enum(type, xitems)}}
{%- endfor -%}
''')

CXX_STRUCT_SERIALIZE_TEMPLATE = Template('''
{%- macro to_json(type, fields, inheritance, fields_size) -%}
inline void to_json(nlohmann::json& Json, const {{ type }}& Type) 
{
{%- for base_type in inheritance %}
	nlohmann::to_json(Json, static_cast<{{base_type}}>(Type));
{% endfor -%}
{%- for field in fields %}
	{%- if field['meta'] == 'string' %}
	if (!CompareStr(Type.{{ field['name'] }}, {{ type }}{}.{{ field['name'] }})) 	
	{% else %}
	if (!(Type.{{ field['name'] }} == {{ type }}{}.{{ field['name'] }})) 	
	{% endif -%}
	{
    {%- if field['meta'] == 'string' %}
		Json["{{ field['name'] }}"] = Type.{{ field['name'] }};
    {% elif field['meta'] == 'bitwise' %}
        to_json_bitwise(Json["{{ field['name'] }}"], Type.{{ field['name'] }});
	{% elif field['meta'] == 'interface' %}
		{%- if field['name'] in fields_size %}
			DeviceObjectReflection::Serialize(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Type.{{ fields_size[field['name']] }});
		{% else %}
			DeviceObjectReflection::Serialize(Json["{{ field['name'] }}"], Type.{{ field['name'] }});
		{% endif -%}	
	{% elif field['name'] in fields_size %}	
		to_json_ptr(Json["{{ field['name'] }}"], Type.{{ field['name'] }}, Type.{{ fields_size[field['name']] }});
	{% elif field['reference'] %}
		to_json_ptr(Json["{{ field['name'] }}"], Type.{{ field['name'] }});
	{% else %}
		Json["{{ field['name'] }}"] = Type.{{ field['name'] }};
	{% endif -%}	
	}
{% endfor -%}
}
{% endmacro -%}

{%- macro from_json(type, fields, inheritance, fields_size) -%}
inline void from_json(const nlohmann::json& Json, {{ type }}& Type)
{
{%- for base_type in inheritance %}
	nlohmann::from_json(Json, static_cast<{{base_type}}&>(Type));
{% endfor -%}
{%- for field in fields %}
	if (Json.contains("{{ field['name'] }}")) 
	{
	{%- if field['meta'] == 'string' %}
		Type.{{ field['name'] }} = copy_string(Json["{{ field['name']}}"].get<std::string>());
    {% elif field['meta'] == 'bitwise' %}
        from_json_bitwise(Json["{{ field['name'] }}"], Type.{{ field['name'] }});
	{% elif field['meta'] == 'interface' %}
		{%- if field['name'] in fields_size %}
			DeviceObjectReflection::Deserialize(Json["{{ field['name'] }}"], &Type.{{ field['name'] }}, Json.at("{{ fields_size[field['name']] }}"));
		{% else %}
			DeviceObjectReflection::Deserialize(Json["{{ field['name'] }}"], &Type.{{ field['name'] }});
		{% endif -%}		
	{% elif field['name'] in fields_size %}
		from_json_ptr(Json["{{ field['name'] }}"], remove_const(&Type.{{ field['name'] }}), Json.at("{{ fields_size[field['name']] }}"));
	{% elif field['reference'] %}
		from_json_ptr(Json["{{ field['name'] }}"], remove_const(&Type.{{ field['name'] }}));
	{% else %}
		Json["{{ field['name'] }}"].get_to(Type.{{ field['name'] }});
	{% endif -%}		
	}
{% endfor -%}
}
{%- endmacro -%}

{%- for type, info in structs %}
{{ to_json(type, info['fields'], info['inheritance'], field_size[type]) }}
{{ from_json(type, info['fields'], info['inheritance'], field_size[type])}}
{% endfor %}
''')

/// @file	
///	@ingroup 	minapi
///	@copyright	Copyright (c) 2016, Cycling '74
///	@license	Usage of this file and its contents is governed by the MIT License

#pragma once

namespace c74 {
namespace min {
	
	
	class texteditor {
	public:
		
		using textfunction = std::function<void(const char*)>;
		
		texteditor(object_base* an_owner, textfunction fn)
		: m_owner		{ an_owner }
		, m_callback	{ fn }
		{}
		
		
		~texteditor() {
			// TODO: Do I need this ????
			// object_free(instance);
		}
		
		
		void open(const char* contents) {
			if (!m_jed) {
				m_jed = max::object_new(max::CLASS_NOBOX, max::gensym("jed"), (max::t_object*)*m_owner, 0);
				object_attr_setsym(m_jed, symbol("title"), symbol("Code Editor"));
				object_attr_setchar(m_jed, symbol("scratch"), 1);
				
				object_method_direct(void, (max::t_object* , const char*, max::t_symbol*),
									 m_jed, symbol("settext"), contents, symbol("utf-8"));
			}
			else
				max::object_attr_setchar(m_jed, max::gensym("visible"), 1);
		}
		
		void open(std::string& contents) {
			open(contents.c_str());
		}
		
		
	private:
		object_base*	m_owner		{ nullptr };
		max::t_object*	m_jed		{ nullptr };
		textfunction	m_callback;

		
		message<> edclose_meth = { m_owner, "edclose", MIN_FUNCTION {
			m_jed = nullptr;
			return {};
		}};

		
		message<> okclose_meth = { m_owner, "okclose", MIN_FUNCTION {
			char* text = nullptr;
			
			object_method(m_jed, max::gensym("gettext"), &text);
			if (text != nullptr) {
				m_callback(text);
				max::sysmem_freeptr(text);
			}
			return {};
		}};

	};
	
}} // namespace c74::min

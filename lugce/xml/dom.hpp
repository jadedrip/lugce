#pragma once
#include <string>
#include <vector>
#include <istream>
#include <exception>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/enable_shared_from_this.hpp>
#include "../string_of.hpp"
#include "simple_sax.hpp"
#include "xml_define.hpp"

namespace lugce
{
	namespace xml
	{
		class not_find : public std::exception
		{
		public:
			virtual const char * what() throw(){ return "The node is not finded"; }
		};

		using namespace boost;

		template< typename _CHAR >
		class document_t;

		/// 元素节点
		template< typename _CHAR >
		class element_t  
		{
			friend class document_t<_CHAR>;
			typedef element_t<_CHAR> element_type;
			typedef typename string_of<_CHAR>::type string_t;
			typedef std::map< string_t, string_t > bond_map_t;
			typedef bond_map_t nspaces_t;
			bool _content_escape;
			string_t _content;			/// 节点内容
		public:
			typedef std::vector< element_t > elements_type;		/// 子元素容器类型
			string_t name;		/// 节点名称
			string_t xmlns;		/// 节点的名字空间 
			elements_type children;		/// 子节点
			bond_map_t xmlns_prefixs;	/// 名字空间对
			bond_map_t attributes;		/// 属性
			enum xml_flag
			{
				xf_null=0, 
				xf_empty_node=1 // 是否输出全空节点
			};
		public:
			element_t() : _content_escape(true){}
			/// 创建一个节点
			element_t( const string_t& n, const string_t&ns=string_t() ) : _content_escape(true), name(n), xmlns(ns){}
		public:	// 子节点
			/// 添加一个子节点
			element_type& append_child( const string_t& name, const string_t& ns=string_t() )
			{
				children.push_back( element_type( name, ns ) );
				return children.back();
			}

			/// 快速获取、添加属性值
			string_t& get_attribute( const string_t& name ){ return attributes[name]; }
			string_t& operator[]( const string_t& name ){ return attributes[name]; }
			string_t& get_attribute( const _CHAR* name ){ return attributes[name]; }
			string_t& operator[]( const _CHAR* name ){ return attributes[name]; }

			/// 获取属性值
			const string_t get_attribute( const string_t& name ) const
			{ 
				typename bond_map_t::const_iterator i=attributes.find( name );
				return i==attributes.end() ? string_t() : i->second;
			}
			const string_t operator[]( const string_t& name ) const{ return get_attribute(name); }
			const string_t get_attribute( const _CHAR* name ) const
			{
				return get_attribute( string_t(name) );
			}
			const string_t operator[]( const _CHAR* name ) const
			{
				return get_attribute( string_t(name) );
			}

			/// 获取指定子节点的内容（找不到不抛出异常，返回空）
			const string_t get_child_content( const string_t& name, const string_t& xs=string_t() ) const throw()
			{
				const element_type *p=find_child( name, xs );
				return p ? p->content() : string_t();
			}

			/// 返回 xml
			const string_t xml( int flag=0 ) const
			{
				std::vector< const element_type* > find_path;
				return do_xml( find_path, flag );
			}

			/// 返回节点内部的 xml
			const string_t inner_xml( int flag=0 ) const
			{
				std::vector< const element_type* > find_path;
				return do_inner_xml( find_path, flag );
			}

			bool valid() const{ return !name.empty(); }

			operator bool() const{ return valid(); }

			/// 返回带名字空间的全名
			const string_t full_name() const{ return xmlns + ':' + name; }

			/// 快速返回节点内容
			string_t& operator *(){ return _content; }
			const string_t& operator *() const{ return _content; }

			/// 设置节点内容，并设置是否进行转义
			void content( const std::string& c, bool escape=true )
			{
				_content=c;
				_content_escape=escape;
			}

			std::string content() const{ return _content; }
		private:
			typedef std::vector< const element_t* > find_path_t;
			const string_t do_inner_xml( find_path_t& fpath, int flag ) const
			{
				string_t s;
				fpath.push_back( this );
				typename elements_type::const_iterator i=children.begin();
				for(;i!=children.end();++i){
					s.append( i->do_xml(fpath, flag) );
				}
				if( _content_escape )
					s.append( details::xml_escape(_content.c_str()) );
				else
					s.append( _content );
				return s;
			}
			
			const string_t do_xml(find_path_t& fpath, int flag) const
			{	// 返回 xml
				if( ( 0==(flag & xf_empty_node) ) && children.empty() && _content.empty() && attributes.empty() )
					return string_t(); // 跳过全空节点
				string_t n=name;
				string_t s; // ='<' + name;
				const element_type* p=fpath.empty() ? NULL : fpath.back();
				bool needns=!p || (p && p->xmlns!=xmlns );	// 是否需要输出名字空间
				// 输出属性
				for(typename bond_map_t::const_iterator i=attributes.begin();i!=attributes.end();++i)
					s+=' ' + i->first + "=\"" + details::xml_escape( i->second.c_str() ) + '"';

				for(typename bond_map_t::const_iterator i=xmlns_prefixs.begin(); i!=xmlns_prefixs.end(); ++i ){
					s+=" xmlns:" + i->first + "=\"" + i->second + '"';
					if( needns && i->second==xmlns ){	// 找到了缩写
						n=i->first + ':' +n;
						needns=false;
					}
				}

				// 在父节点中查询缩写
				for( typename find_path_t::const_reverse_iterator i=fpath.rbegin(); i!=fpath.rend(); ++i ){
					if( needns ){
						for(typename bond_map_t::const_iterator i=p->xmlns_prefixs.begin(); i!=p->xmlns_prefixs.end(); ++i ){
							if( i->second==xmlns ){	// 找到了缩写
								n=i->first + ':' +n;
								needns=false;
								break;
							}
						}
					}
				}

				if( needns && !xmlns.empty() )
					s+=" xmlns=\"" + xmlns + '"';

				const string_t inner=inner_xml(flag);
				if( inner.empty() )
					return '<' + n + s + "/>";
				else
					return '<' + n + s + '>' + inner + "</" + n + '>';
			}
		public:
			/// 从当前节点开始，查询第一个指定名称的子节点
			const element_type* find_child( const string_t& xpath, const string_t& ns=string_t() ) const
			{
				if( xpath.empty() || children.empty() ) return NULL;
				string_t ns_=ns.empty() ? xmlns : ns;
				typename elements_type::const_iterator i=children.begin();
				bool u=(i->xmlns==xmlns && ns.empty());
				for(;i!=children.end();++i){
					if( ( i->name==xpath ) && ( u || (i->xmlns==ns) ) )
						return &(*i);
				}
				return NULL;
			}

			/// 从当前节点开始，查询第一个指定名称的子节点
			element_type* find_child( const string_t& xpath, const string_t& ns=string_t() )
			{
				if( xpath.empty() || children.empty() ) return NULL;
				typename elements_type::iterator i=children.begin();
				bool u=(i->xmlns==xmlns && ns.empty());
				for(;i!=children.end();++i){
					if( ( i->name==xpath ) && ( u || (i->xmlns==ns) ) )
						return &(*i);
				}
				return NULL;
			}

			/// 快速获取子节点
			const element_type& operator / ( const string_t& xpath ) const
			{
				typename string_t::size_type i=xpath.find_first_of(':');
				const element_type* p;
				if( i==string_t::npos )
					p=find_child(xpath);	
				else
					p=find_child(xpath.substr(i+1),xpath.substr(0,i));
				if( !p ) throw not_find();
				return *p;
			}

			/// 快速获取并复制子节点
			/// <remark>
			/// 如果找不到，不抛出异常而返回一个空节点。另外，节点是被复制出来的，
			/// 因此可能对效率有所影响。
			/// </remark>
			const element_type operator /= ( const string_t& xpath ) const
			{
				typename string_t::size_type i=xpath.find_first_of(':');
				const element_type* p;
				if( i==string_t::npos )
					p=find_child(xpath);	
				else
					p=find_child(xpath.substr(i+1),xpath.substr(0,i));
				return p ? *p : elements_type();
			}

			/// 快速获取子节点，如果没有找到，会创建一个新节点
			element_type& operator / ( const string_t& xpath )
			{
				typename string_t::size_type i=xpath.find_last_of(':');
				element_type* p;
				if( i==string_t::npos )
					p=find_child(xpath);	
				else
					p=find_child(xpath.substr(i+1),xpath.substr(0,i));
				if( p ) return *p;
				// 如果没有找到，创建这个节点
				if( i==string_t::npos )
					children.push_back( element_type( xpath ) );
				else
					children.push_back( element_type( xpath.substr(i+1),xpath.substr(0,i) ) );
				return children.back();
			}
		};

		template< typename _CHAR >
		class document_t : public element_t<_CHAR>
		{
		public:
			typedef element_t<_CHAR> element_type;
			typedef document_t<_CHAR> document_type;
		private:
			typedef typename string_of<_CHAR>::type string_t;
		private:
			void init_sax()
			{
				_sax.bind_processor( boost::bind(&document_type::HandleProcessor, this, _1, _2) );
				_sax.bind_element_start( boost::bind(&document_type::HandleElementStart, this,_1, _2) );
				_sax.bind_element_end( boost::bind(&document_type::HandleElementEnd, this,_1) );
				_sax.bind_content( boost::bind(&document_type::HandleElementContent, this,_1) );
				_sax.bind_dtd( boost::bind(&document_type::HandleElementDTD, this, _1, _2) );
			}
		public:
			string_t version;
			string_t encoding;
		public:
			document_t( const string_t& version_, const string_t& encoding_, const string_t& root_name, const string_t& ns=string_t() )
				: version( version_ ), encoding( encoding_ )
			{
				this->name=root_name;
				this->xmlns=ns;
			}
			document_t(){ init_sax(); }
			document_t( const string_t& x ){ init_sax(); xml(x); }
			document_t( std::istream& x ){ init_sax(); load(x); }

			element_type& root()
			{ 
				return *this;
			}
			const element_type& root() const
			{ 
				return *this;
			}

			/// 创建根节点
			element_type& create_root( const string_t& root_name, const string_t& ns=string_t() )
			{ 
				this->name=root_name;
				this->xmlns=ns;
				return *this;
			}

			void load( std::istream& st )
			{
				_current=NULL;
				_sax.load(st);
			}

			void xml( const string_t& xml_str )
			{
				_current=NULL;
				_sax.load( xml_str, true );
			}

			const string_t xml( int flag=0 ) const
			{
				string_t x;
				xml( &x );
				return x;
			}

			void xml( string_t* x, int flag=0 ) const
			{
				assert(x);
				if( !version.empty() || !encoding.empty() ){
					*x="<?xml version=\"";
					if( version.empty() ){ 
						*x+="1.0\" encoding=\"" + encoding;
					}else{
						*x+=version+'"';
						if( !encoding.empty() )
							*x+=" encoding=\"" + encoding;
					}
					*x+="\"?>";
				}
				x->append( element_t<_CHAR>::xml( flag ) );
			}
		private:
			typedef std::map< string_t, string_t > bond_map_t;
			static const string_t attribute( bond_map_t& attrs, const string_t& name )
			{
				typename bond_map_t::const_iterator i=attrs.find( name );
				return i==attrs.end() ? string_t() : i->second;
			}
		private:
			void get_name_ns( const string_t& name, string_t& n, string_t& ns )
			{
				typename string_t::size_type i=name.find_first_of(':');
				if( i==string_t::npos ){
					n=name;
				}else{
					n=name.substr(i+1);
					ns=get_namespace( name.substr(0,i) );
				}
			}

			void HandleProcessor( const string_t& name, bond_map_t& attrs )
			{
				if( name=="xml" ){
					version=attribute(attrs, "version");
					encoding=attribute(attrs, "encoding");
				}
			}

			void HandleElementStart( const string_t& name, bond_map_t& attrs )
			{
				string_t n, ns;
				get_name_ns( name, n, ns );
				if( ns.empty() ) ns=attribute(attrs, "xmlns");

				if( _current==NULL ){
					this->name=n;
					this->xmlns=ns;
					_current=this;
				}else{
					_current->children.push_back( element_type( n, ns ) );
					_nodes.push_back( _current );
					_current=&(_current->children.back());
				}

				for( typename bond_map_t::const_iterator x=attrs.begin(); x!=attrs.end(); ++x ){
					if( x->first=="xmlns" ) continue;
					if( x->first.substr(0,6)=="xmlns:" ){	// 设置名字空间缩写
						_current->xmlns_prefixs[ x->first.substr(6) ]=x->second;	
					}else{
						_current->attributes[x->first]=x->second;
					}
				}
			}

			void HandleElementEnd( const string_t& name )
			{
				string_t n, ns;
				get_name_ns( name, n, ns );
				if( !_current || _current->name!=n ) 
					throw bad_format();
				if( _nodes.empty() ){	//TODO: 文档结束
				}else{
					_current=_nodes.back();
					_nodes.erase( _nodes.end()-1 );
				}
			}

			void HandleElementContent( const string_t& name )
			{
				if( !_current ) 
					throw bad_format();
				_current->_content.append( name );
			}

			void HandleElementDTD( const string_t& name, const string_t& value )
			{
				std::cout << "DTD: " << name << " " << value << std::endl;
			}

			const string_t get_namespace( const string_t& prefix )
			{
				if( !_current ) return string_t();
				typename bond_map_t::const_iterator i=_current->xmlns_prefixs.find( prefix );
				if( i!=_current->xmlns_prefixs.end() ) return i->second;
				for( typename std::vector<element_type*>::const_reverse_iterator x=_nodes.rbegin();x!=_nodes.rend(); ++x ){
					element_type* p=*x;
					if( !p ) continue;
					i=p->xmlns_prefixs.find( prefix );
					if( i!=p->xmlns_prefixs.end() ) return i->second;
				}
				return string_t();
			}
		private:
			std::vector< element_type* > _nodes;
			simple_sax _sax;
			element_type* _current;
		};
		typedef document_t<char> document;
		typedef document_t<wchar_t> wdocument;
		typedef element_t<char> element;
		typedef element_t<wchar_t> welement;
	};
};

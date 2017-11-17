#pragma once

struct null_t {};

template <class _class_t, class _parent_class_t, bool _is_unique>
struct _Identifier
{
	typedef _class_t			current_class_t;
	typedef _parent_class_t		parent_class_t;

	enum { is_unique = _is_unique };
};

template <class _class_t>
struct is_unique_class
{
	typedef typename _class_t::_identifier_t		identifier_t;
	typedef typename identifier_t::parent_class_t	parent_t;

	enum
	{
		value = std::_If<identifier_t::is_unique, std::true_type, is_unique_class<parent_t>>::type::value
	};
};

template <>
struct is_unique_class<null_t>
{
	enum { value = false };
};


template <class _class_t>
struct get_unique_class
{
	typedef typename _class_t::_identifier_t		identifier_t;
	typedef typename identifier_t::parent_class_t	parent_t;

	typedef typename std::_If<identifier_t::is_unique, _class_t, typename get_unique_class<parent_t>::type>::type type;
};

template <>
struct get_unique_class<null_t>
{
	typedef null_t type;
};


template <class _class_t>
struct get_class
{
	typedef typename std::_If<is_unique_class<_class_t>::value, typename get_unique_class<_class_t>::type, _class_t>::type type;
};
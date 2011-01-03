#ifndef V3D_PYTHON_CONVERT_QLIST_H_
#define V3D_PYTHON_CONVERT_QLIST_H_

/*
 * convert_qlist.h
 *
 *  Created on: Jan 03, 2011
 *      Author: Christopher M. Bruns
 *
 *  Adapted from
 *    http://www.boost.org/doc/libs/1_42_0/libs/python/doc/v2/faq.html#custom_string
 *
 *    This header included boost/python, and thus should NOT be included
 *    by anything parsed by gccxml in pyplusplus.
 */

#include <boost/python/module.hpp>
#include <boost/python/def.hpp>
#include <boost/python/to_python_converter.hpp>
#include <QList>

namespace sandbox { namespace {

template<class ELT>
struct qlist_to_python_list
{
    static PyObject* convert(QList<ELT> const& list)
    {
        boost::python::list pyList;
        // we need to wrap the pointers into PyObjects
        typename boost::python::reference_existing_object::apply<ELT>::type converter;
        for (int i = 0; i < list.size(); ++i)
        {
            pyList.append( boost::python::object(list[i]) );
        }
        return boost::python::incref( pyList.ptr() );
    }
};

template<class ELT>
struct qlist_from_python_list
{
    qlist_from_python_list()
    {
        boost::python::converter::registry::push_back(
                &convertible,
                &construct,
                boost::python::type_id<QList<ELT> >());
    }

    static void* convertible(PyObject* obj_ptr)
    {
        if (!PySequence_Check(obj_ptr))
            return 0;

        if( !PyObject_HasAttrString( obj_ptr, "__len__" ) ) {
            return 0;
        }

        boost::python::object py_sequence(
                bp::handle<>( bp::borrowed( obj_ptr ) ) );
        // Ensure each element is of the correct type (ELT)
        int n = PySequence_Size(obj_ptr);
        for (Py_ssize_t i = 0; i < n; ++i)
        {
            if (! boost::python::extract<ELT>(py_sequence[i]).check() )
                return 0;
        }

        return obj_ptr;
    }

    static void construct(
            PyObject* obj_ptr,
            boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        boost::python::object pyList(boost::python::handle<>(boost::python::borrowed(obj_ptr)));

        void* storage = (
                (boost::python::converter::rvalue_from_python_storage<QList<ELT> >*)
                data)->storage.bytes;
        new (storage) QList<ELT>();

        QList<ELT> *container = static_cast<QList<ELT>* >(storage);
        int n = PySequence_Size(obj_ptr);
        for (int i = 0; i < n; ++i)
            container->append( boost::python::extract<ELT>(pyList[i]) );

        data->convertible = storage;
    }
};

}} // namespace sandbox::<anonymous>

template<class ELT>
void register_qlist_conversion()
{
    boost::python::to_python_converter<
            QList<ELT>,
            sandbox::qlist_to_python_list<ELT> >();

    sandbox::qlist_from_python_list<ELT>();
}

#endif // V3D_PYTHON_CONVERT_QLIST_H_

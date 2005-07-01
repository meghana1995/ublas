//
//  Copyright (c) 2000-2002
//  Joerg Walter, Mathias Koch
//
//  Permission to use, copy, modify, distribute and sell this software
//  and its documentation for any purpose is hereby granted without fee,
//  provided that the above copyright notice appear in all copies and
//  that both that copyright notice and this permission notice appear
//  in supporting documentation.  The authors make no representations
//  about the suitability of this software for any purpose.
//  It is provided "as is" without express or implied warranty.
//
//  The authors gratefully acknowledge the support of
//  GeNeSys mbH & Co. KG in producing this work.
//

#ifndef _BOOST_UBLAS_EXPRESSION_TYPE_
#define _BOOST_UBLAS_EXPRESSION_TYPE_

#include <boost/numeric/ublas/exception.hpp>
#include <boost/numeric/ublas/traits.hpp>
#include <boost/numeric/ublas/functional.hpp>


// Expression templates based on ideas of Todd Veldhuizen and Geoffrey Furnish
// Iterators based on ideas of Jeremy Siek

namespace boost { namespace numeric { namespace ublas {

    // Base class for uBLAS staticaly derived expressions - see the Barton Nackman trick
    //  Provides numeric properties for linear algebra
    template<class E>
    class ublas_expression {
    public:
        typedef E expression_type;
        /* E can be an incomplete type - to define the following we would need more template arguments
        typedef typename E::type_category type_category;
        typedef typename E::value_type value_type;
        */
        
        // Directly implement nonassignable - simplifes debugging call trace!
    protected:
        ublas_expression () {}
        ~ublas_expression () {}
    private:
        const ublas_expression& operator= (const ublas_expression &);
    };


    // Base class for Scalar Expression models -
    //  it does not model the Scalar Expression concept but all derived types should.
    // The class defines a common base type and some common interface for all
    // statically derived Scalar Expression classes
    // We implement the casts to the statically derived type.
    template<class E>
    class scalar_expression:
        public ublas_expression<E> {
    public:
        typedef E expression_type;
        typedef scalar_tag type_category;

        BOOST_UBLAS_INLINE
        const expression_type &operator () () const {
            return *static_cast<const expression_type *> (this);
        }
        BOOST_UBLAS_INLINE
        expression_type &operator () () {
            return *static_cast<expression_type *> (this);
        }
    };

    template<class T>
    class scalar_reference:
        public scalar_expression<scalar_reference<T> > {

        typedef scalar_reference<T> self_type;
    public:
        typedef T value_type;
        typedef const value_type &const_reference;
        typedef typename boost::mpl::if_<boost::is_const<T>,
                                          const_reference,
                                          value_type &>::type reference;
        typedef const self_type const_closure_type;
        typedef const_closure_type closure_type;

        // Construction and destruction
        BOOST_UBLAS_INLINE
        scalar_reference ():
            t_ (nil_) {}
        BOOST_UBLAS_INLINE
        explicit scalar_reference (reference t):
            t_ (t) {}

        // Conversion
        BOOST_UBLAS_INLINE
        operator value_type () const {
            return t_;
        }

        // Assignment
        BOOST_UBLAS_INLINE
        scalar_reference &operator = (const scalar_reference &s) {
            t_ = s.t_;
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        scalar_reference &operator = (const scalar_expression<AE> &ae) {
            t_ = ae;
            return *this;
        }

        // Closure comparison
        BOOST_UBLAS_INLINE
        bool same_closure (const scalar_reference &sr) const {
            return &t_ == &sr.t_;
        }

    private:
        reference t_;
        static value_type nil_;
    };

    template<class T>
    typename scalar_reference<T>::value_type scalar_reference<T>::nil_
        = typename scalar_reference<T>::value_type ();

    template<class T>
    class scalar_value:
        public scalar_expression<scalar_value<T> > {

        typedef scalar_value<T> self_type;
    public:
        typedef T value_type;
        typedef const value_type &const_reference;
        typedef typename boost::mpl::if_<boost::is_const<T>,
                                          const_reference,
                                          value_type &>::type reference;
        typedef const scalar_reference<const self_type> const_closure_type;
        typedef scalar_reference<self_type> closure_type;

        // Construction and destruction
        BOOST_UBLAS_INLINE
        scalar_value ():
            t_ () {}
        BOOST_UBLAS_INLINE
        scalar_value (const value_type &t):
            t_ (t) {}

        BOOST_UBLAS_INLINE
        operator value_type () const {
            return t_;
        }

        // Assignment
        BOOST_UBLAS_INLINE
        scalar_value &operator = (const scalar_value &s) {
            t_ = s.t_;
            return *this;
        }
        template<class AE>
        BOOST_UBLAS_INLINE
        scalar_value &operator = (const scalar_expression<AE> &ae) {
            t_ = ae;
            return *this;
        }

        // Closure comparison
        BOOST_UBLAS_INLINE
        bool same_closure (const scalar_value &sv) const {
            return this == &sv;    // self closing on instances value
        }

    private:
        value_type t_;
    };


    // Base class for Vector Expression models -
    //  it does not model the Vector Expression concept but all derived types should.
    // The class defines a common base type and some common interface for all
    // statically derived Vector Expression classes
    // We implement the casts to the statically derived type.
    template<class E>
    class vector_expression:
        public ublas_expression<E> {
    public:
        static const unsigned complexity = 0;
        typedef E expression_type;
        typedef vector_tag type_category;
        /* E can be an incomplete type - to define the following we would need more template arguments
        typedef typename E::size_type size_type;
        */
 
        BOOST_UBLAS_INLINE
        const expression_type &operator () () const {
            return *static_cast<const expression_type *> (this);
        }
        BOOST_UBLAS_INLINE
        expression_type &operator () () {
            return *static_cast<expression_type *> (this);
        }

    private:
        // projection types
        typedef vector_range<E> vector_range_type;
        typedef vector_slice<E> vector_slice_type;
        // vector_indirect_type will depend on the A template parameter 
        typedef basic_range<> default_range;
        typedef basic_slice<> default_slice;
   public:
        // projection functions - projects must be constructable from default size_t, range and slice types
        BOOST_UBLAS_INLINE
        const vector_range_type range (std::size_t start, std::size_t stop) const {
            return vector_range_type (operator () (), default_range (start, stop));
        }
        BOOST_UBLAS_INLINE
        vector_range_type range (std::size_t start, std::size_t stop) {
            return vector_range_type (operator () (), default_range (start, stop));
        }
#ifdef BOOST_UBLAS_ENABLE_PROXY_SHORTCUTS
        BOOST_UBLAS_INLINE
        const vector_range_type operator () (const default_range &r) const {
            return vector_range_type (operator () (), r);
        }
        BOOST_UBLAS_INLINE
        vector_range_type operator () (const default_range &r) {
            return vector_range_type (operator () (), r);
        }
        BOOST_UBLAS_INLINE
        const vector_slice_type operator () (const default_slice &s) const {
            return vector_slice_type (operator () (), s);
        }
        BOOST_UBLAS_INLINE
        vector_slice_type operator () (const default_slice &s) {
            return vector_slice_type (operator () (), s);
        }
        template<class A>
        BOOST_UBLAS_INLINE
        const vector_indirect<const E, A> operator () (const indirect_array<A> &ia) const {
            return vector_indirect<const E, A>  (operator () (), ia);
        }
        template<class A>
        BOOST_UBLAS_INLINE
        vector_indirect<E, A> operator () (const indirect_array<A> &ia) {
            return vector_indirect<E, A> (operator () (), ia);
        }
#endif
        BOOST_UBLAS_INLINE
        const vector_range_type project (const default_range &r) const {
            return vector_range_type (operator () (), r);
        }
        BOOST_UBLAS_INLINE
        vector_range_type project (const default_range &r) {
            return vector_range_type (operator () (), r);
        }
        BOOST_UBLAS_INLINE
        const vector_slice_type project (const default_slice &s) const {
            return vector_slice_type (operator () (), s);
        }
        BOOST_UBLAS_INLINE
        vector_slice_type project (const default_slice &s) {
            return vector_slice_type (operator () (), s);
        }
        template<class A>
        BOOST_UBLAS_INLINE
        const vector_indirect<const E, A> project (const indirect_array<A> &ia) const {
            return vector_indirect<const E, A> (operator () (), ia);
        }
        template<class A>
        BOOST_UBLAS_INLINE
        vector_indirect<E, A> project (const indirect_array<A> &ia) {
            return vector_indirect<E, A> (operator () (), ia);
        }
    };

    // Base class for Vector container models -
    //  it does not model the Vector concept but all derived types should.
    // The class defines a common base type and some common interface for all
    // statically derived Vector classes
    // We implement the casts to the statically derived type.
    template<class C>
    class vector_container:
        public vector_expression<C> {
    public:
        static const unsigned complexity = 0;
        typedef C container_type;
        typedef vector_tag type_category;
 
        BOOST_UBLAS_INLINE
        const container_type &operator () () const {
            return *static_cast<const container_type *> (this);
        }
        BOOST_UBLAS_INLINE
        container_type &operator () () {
            return *static_cast<container_type *> (this);
        }

#ifdef BOOST_UBLAS_ENABLE_PROXY_SHORTCUTS
        using vector_expression<C>::operator ();
#endif
    };


    // Base class for Matrix Expression models -
    //  it does not model the Matrix Expression concept but all derived types should.
    // The class defines a common base type and some common interface for all
    // statically derived Matrix Expression classes
    // We implement the casts to the statically derived type.
    template<class E>
    class matrix_expression:
        public ublas_expression<E> {
    public:
        static const unsigned complexity = 0;
        typedef E expression_type;
        typedef matrix_tag type_category;
        /* E can be an incomplete type - to define the following we would need more template arguments
        typedef typename E::size_type size_type;
        */

        BOOST_UBLAS_INLINE
        const expression_type &operator () () const {
            return *static_cast<const expression_type *> (this);
        }
        BOOST_UBLAS_INLINE
        expression_type &operator () () {
            return *static_cast<expression_type *> (this);
        }

    private:
        // projection types
        typedef noalias_proxy<E> noalias_proxy_type;
        typedef const matrix_row<const E> const_matrix_row_type;
        typedef matrix_row<E> matrix_row_type;
        typedef const matrix_column<const E> const_matrix_column_type;
        typedef matrix_column<E> matrix_column_type;
        typedef const matrix_range<const E> const_matrix_range_type;
        typedef matrix_range<E> matrix_range_type;
        typedef const matrix_slice<const E> const_matrix_slice_type;
        typedef matrix_slice<E> matrix_slice_type;
        typedef const matrix_indirect<const E> const_matrix_indirect_type;
        typedef matrix_indirect<E> matrix_indirect_type;

    public:
        BOOST_UBLAS_INLINE
        noalias_proxy_type noalias () {
            return noalias_proxy_type (operator () ());
        }
        BOOST_UBLAS_INLINE
        const_matrix_row_type operator [] (std::size_t i) const {
            return const_matrix_row_type (operator () (), i);
        }
        BOOST_UBLAS_INLINE
        matrix_row_type operator [] (std::size_t i) {
            return matrix_row_type (operator () (), i);
        }
        BOOST_UBLAS_INLINE
        const_matrix_row_type row (std::size_t i) const {
            return const_matrix_row_type (operator () (), i);
        }
        BOOST_UBLAS_INLINE
        matrix_row_type row (std::size_t i) {
            return matrix_row_type (operator () (), i);
        }
        BOOST_UBLAS_INLINE
        const_matrix_column_type column (std::size_t j) const {
            return const_matrix_column_type (operator () (), j);
        }
        BOOST_UBLAS_INLINE
        matrix_column_type column (std::size_t j) {
            return matrix_column_type (operator () (), j);
        }
#ifndef BOOST_UBLAS_NO_PROXY_SHORTCUTS
        BOOST_UBLAS_INLINE
        const_matrix_range_type operator () (const range &r1, const range &r2) const {
            return const_matrix_range_type (operator () (), r1, r2);
        }
        BOOST_UBLAS_INLINE
        matrix_range_type operator () (const range &r1, const range &r2) {
            return matrix_range_type (operator () (), r1, r2);
        }
        BOOST_UBLAS_INLINE
        const_matrix_slice_type operator () (const slice &s1, const slice &s2) const {
            return const_matrix_slice_type (operator () (), s1, s2);
        }
        BOOST_UBLAS_INLINE
        matrix_slice_type operator () (const slice &s1, const slice &s2) {
            return matrix_slice_type (operator () (), s1, s2);
        }
        template<class A>
        BOOST_UBLAS_INLINE
        const_matrix_indirect_type operator () (const indirect_array<A> &ia1, const indirect_array<A> &ia2) const {
            return const_matrix_indirect_type (operator () (), ia1, ia2);
        }
        template<class A>
        BOOST_UBLAS_INLINE
        matrix_indirect_type operator () (const indirect_array<A> &ia1, const indirect_array<A> &ia2) {
            return matrix_indirect_type (operator () (), ia1, ia2);
        }
#else
        BOOST_UBLAS_INLINE
        const_matrix_range_type project (const range &r1, const range &r2) const {
            return const_matrix_range_type (operator () (), r1, r2);
        }
        BOOST_UBLAS_INLINE
        matrix_range_type project (const range &r1, const range &r2) {
            return matrix_range_type (operator () (), r1, r2);
        }
        BOOST_UBLAS_INLINE
        const_matrix_slice_type project (const slice &s1, const slice &s2) const {
            return const_matrix_slice_type (operator () (), s1, s2);
        }
        BOOST_UBLAS_INLINE
        matrix_slice_type project (const slice &s1, const slice &s2) {
            return matrix_slice_type (operator () (), s1, s2);
        }
        template<class A>
        BOOST_UBLAS_INLINE
        const_matrix_indirect_type project (const indirect_array<A> &ia1, const indirect_array<A> &ia2) const {
            return const_matrix_indirect_type (operator () (), ia1, ia2);
        }
        template<class A>
        BOOST_UBLAS_INLINE
        matrix_indirect_type project (const indirect_array<A> &ia1, const indirect_array<A> &ia2) {
            return matrix_indirect_type (operator () (), ia1, ia2);
        }
#endif
    };

#ifdef BOOST_UBLAS_NO_NESTED_CLASS_RELATION
    struct iterator1_tag {};
    struct iterator2_tag {};

    template<class I>
    BOOST_UBLAS_INLINE
    typename I::dual_iterator_type begin (const I &it, iterator1_tag) {
        return it ().find2 (1, it.index1 (), 0);
    }
    template<class I>
    BOOST_UBLAS_INLINE
    typename I::dual_iterator_type end (const I &it, iterator1_tag) {
        return it ().find2 (1, it.index1 (), it ().size2 ());
    }
    template<class I>
    BOOST_UBLAS_INLINE
    typename I::dual_reverse_iterator_type rbegin (const I &it, iterator1_tag) {
        return typename I::dual_reverse_iterator_type (end (it, iterator1_tag ()));
    }
    template<class I>
    BOOST_UBLAS_INLINE
    typename I::dual_reverse_iterator_type rend (const I &it, iterator1_tag) {
        return typename I::dual_reverse_iterator_type (begin (it, iterator1_tag ()));
    }

    template<class I>
    BOOST_UBLAS_INLINE
    typename I::dual_iterator_type begin (const I &it, iterator2_tag) {
        return it ().find1 (1, 0, it.index2 ());
    }
    template<class I>
    BOOST_UBLAS_INLINE
    typename I::dual_iterator_type end (const I &it, iterator2_tag) {
        return it ().find1 (1, it ().size1 (), it.index2 ());
    }
    template<class I>
    BOOST_UBLAS_INLINE
    typename I::dual_reverse_iterator_type rbegin (const I &it, iterator2_tag) {
        return typename I::dual_reverse_iterator_type (end (it, iterator2_tag ()));
    }
    template<class I>
    BOOST_UBLAS_INLINE
    typename I::dual_reverse_iterator_type rend (const I &it, iterator2_tag) {
        return typename I::dual_reverse_iterator_type (begin (it, iterator2_tag ()));
    }
#endif

    // Base class for Matrix container models -
    //  it does not model the Matrix concept but all derived types should.
    // The class defines a common base type and some common interface for all
    // statically derived Matrix classes
    // We implement the casts to the statically derived type.
    template<class C>
    class matrix_container:
        public matrix_expression<C> {
    public:
        static const unsigned complexity = 0;
        typedef C container_type;
        typedef matrix_tag type_category;

        BOOST_UBLAS_INLINE
        const container_type &operator () () const {
            return *static_cast<const container_type *> (this);
        }
        BOOST_UBLAS_INLINE
        container_type &operator () () {
            return *static_cast<container_type *> (this);
        }

#ifdef BOOST_UBLAS_ENABLE_PROXY_SHORTCUTS
        using matrix_expression<C>::operator ();
#endif
    };

}}}

#endif

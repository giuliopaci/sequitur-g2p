/*
 * $Id: ReferenceCounting.hh 95 2007-06-02 14:32:35Z max $
 *
 * Copyright (c) 2004-2005  RWTH Aachen University
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 2 (June
 * 1991) as published by the Free Software Foundation.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, you will find it at
 * http://www.gnu.org/licenses/gpl.html, or write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110,
 * USA.
 *
 * Should a provision of no. 9 and 10 of the GNU General Public License
 * be invalid or become invalid, a valid provision is deemed to have been
 * agreed upon which comes closest to what the parties intended
 * commercially. In any case guarantee/warranty shall be limited to gross
 * negligent actions or intended actions or fraudulent concealment.
 */

#ifndef _CORE_REFERENCE_COUNTING
#define _CORE_REFERENCE_COUNTING

#include "Assertions.hh"
#include "Types.hh"

namespace Core {

    template <class T> class Ref;

    /**
     * Base class for reference-counted objects.
     *
     * To enable reference counting for a class it need to inherit this
     * class publicly.
     */

    class ReferenceCounted {
    private:
	template <class T> friend class Ref;
	mutable u32 referenceCount_;
	explicit ReferenceCounted(u32 rc) : referenceCount_(rc) {}

	static inline ReferenceCounted *sentinel() {
	    static ReferenceCounted sentinel_(1);
	    return &sentinel_;
	}
	static bool isSentinel(const ReferenceCounted *object) {
	    return object == sentinel();
	}
	static bool isNotSentinel(const ReferenceCounted *object) {
	    return object != sentinel();
	}
    public:
	ReferenceCounted() : referenceCount_(0) {}
	ReferenceCounted(const ReferenceCounted&) : referenceCount_(0) {}
	ReferenceCounted &operator= (const ReferenceCounted&) { return *this; }

	u32 refCount() const { return referenceCount_; }
	void free() const {
	    require_(!referenceCount_);
	    verify_(!isSentinel(this));
	    delete this;
	}
    };

    /**
     * Class template for smart pointers using intrusive
     * reference-counting.
     *
     * Naming: Concrete smart pointer types should have a "Ref" suffix:
     * e.g. typedef Ref<Foo> FooRef;
     * (Obviously calling this class "Ref" instead of more
     * correctly "Reference" deviates from the naming conventions.
     * However since this class is used extensively a shorter name is
     * mandated.  "Ref" is analogous to the "REF" keyword in
     * Modula-3.)
     *
     * @c T should be a descendant of ReferenceCounted.
     *
     * Note: Virtual derivation from ReferenceCounted is not allowed
     * currently.  Reasons are many but the most important one is
     * that we use a sentinel instead of null to represent a reference
     * is invalid (i.e. void).  Advantage of using a sentinel is that
     * in the most frequently used functions (e.g. copy constructor)
     * the sentinel can be treated as if it was a normal object.  Thus
     * we save a few if-clauses.  Once we use a sentinel, the static
     * sentinel object (delivered by ReferenceCounted::sentinel()) of
     * type ReferenceCounted needs to get casted to the type
     * Object. It is a dirty solution but is the only way to make the
     * usage of the sentinel fast.  In order to be independent of the
     * order of derivation in case of multiple inheritance - i.e. if
     * ReferenceCounted is the first precursor class or not -
     * static_cast is used to cast the sentinel to the type Object.
     * Usage of reinterpret_cast would work only if we could ensure
     * that ReferenceCounted was the very first class in the type
     * Object.  Finally, closing the series of arguments, virtual
     * derivation from ReferenceCounted is not permitted by
     * static_cast.  It does not mean that ReferenceCounting and
     * virtual inheritance does not go along at all: Make your virtual
     * inheritance hierarchy first, and derive only the most specific
     * class from ReferenceCounted.
     */

    template <class T /* -> ReferenceCounted */>
    class Ref {
    private:
	typedef T Object;
	Object *object_;
	static Object* sentinel() { return static_cast<Object*>(Object::sentinel()); }

	/**
	 * Takes over the value of @param o.
	 * - Correctly handles self assignment, i.e. if @param o ==
	 *   object_.
	 * - @see the function ref() for how to create a Ref object
	 *   conveniently from a pointer.
	 */
	void set(Object *o) {
	    const Object *old = object_;
	    ++(object_ = o)->referenceCount_;
	    if (!(--old->referenceCount_)) {
		verify_(Object::isNotSentinel(old));
		old->free();
	    }
	}
    public:
	Ref() : object_(sentinel()) { ++object_->referenceCount_; }

	explicit Ref(Object *o) : object_(o) {
	    require_(o);
	    ++object_->referenceCount_;
	}

	/**
	 * Copy Constructor for the Type Ref<T>
	 *
	 * @warning: Implementation of this constructor is necessary
	 * although the template copy constructor implements exactly
	 * the same function.  If this constructor is not defined, the
	 * compiler creates a default copy constructor instead of
	 * using the template one.
	 */
	Ref(const Ref &r) : object_(r.object_) {
	    ++object_->referenceCount_;
	}
	/**
	 * Template Copy Constructor.
	 *
	 * Using a template allows automatic conversion in following cases:
	 * - from non-const to const reference
	 * - from derived to precursor class reference
	 * For all classes S which are not derived from T, this
	 * constructor will fail to compile.
	 */
	template<class S> Ref(const Ref<S> &r) : object_(r._get()) {
	    ++object_->referenceCount_;
	}

	~Ref() {
	    if (!(--object_->referenceCount_)) {
		verify_(Object::isNotSentinel(object_));
		object_->free();
	    }
	}

	Ref &operator=(const Ref &rhs) {
	    set(rhs.object_);
	    return *this;
	}
	template<class S> Ref &operator=(const Ref<S> &rhs) {
	    set(rhs._get());
	    return *this;
	}

	/**
	 * Reset to void.
	 * Corresponds to assigning zero to normal pointer.
	 */
	void reset() {
	    if (Object::isNotSentinel(object_)) {
		if (!(--object_->referenceCount_)) {
		    object_->free();
		}
		++(object_ = sentinel())->referenceCount_;
	    }
	}

	/** Test for identity */
	bool operator==(const Ref &r) const {
	    return object_ == r.object_;
	}
	template<class S> bool operator==(const Ref<S> &r) const {
	    return object_ == r._get();
	}
	/** Test for non-identity */
	bool operator!=(const Ref &r) const {
	    return object_ != r.object_;
	}
	template<class S> bool operator!=(const Ref<S> &r) const {
	    return object_ != r._get();
	}

	/** Test for non-voidness. */
	operator bool() const {
	    return Object::isNotSentinel(object_);
	}
	/** Test for voidness. */
	bool operator!() const {
	    return Object::isSentinel(object_);
	}

	Object &operator* () const {
	    require_(Object::isNotSentinel(object_));
	    return *object_;
	}

	Object *operator-> () const {
	    require_(Object::isNotSentinel(object_));
	    return object_;
	}

	/** Explicit conversion to normal pointer.
	 * @warning This defeats the reference counting mechanism.
	 * Use with caution!
	 */
	Object* get() const {
	    return Object::isNotSentinel(object_) ? object_ : 0;
	}
	/** Value of internal pointer.
	 * @warning Do not use this function. It is only used in the
	 * template copy constructor and the template operator= functions.
	 */
	Object* _get() const { return object_; }
    };

    /** Convenience constructors for Ref smart pointers. */
    template <class T>
    inline Ref<T> ref(T *o) { return Ref<T>(o); }

} // Core

#endif // _CORE_REFERENCE_COUNTING

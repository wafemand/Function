#pragma once

#include <functional>
#include <memory>


template<typename>
class Function;


template<typename R, typename ...Args>
struct Function<R(Args...)> {

    static const int BUF_SIZE = 42;

    Function(std::nullptr_t) : concept() {
        isSmall = false;
    }

    Function(Function const &other) {
        isSmall = other.isSmall;
        if (isSmall) {
            other.get_concept()->placement_copy_small(buf);
        } else {
            other.get_concept()->placement_copy_big(buf);
        }
    }

    Function(Function &&other) {
        if (other.isSmall) {
            other.get_concept()->placement_move(buf);
        } else {
            concept = std::move(other.concept);
        }
        isSmall = std::move(other.isSmall);
    }

    template<typename F>
    Function(F f) : concept(nullptr) {
        isSmall = sizeof(f) < BUF_SIZE;
        if (isSmall) {
            new(buf) Model<F>(std::move(f));
        } else {
            concept = std::make_unique<Model < F> > (std::move(f));
        }
    }

    void destroy() {
        if (isSmall) {
            get_concept()->~Concept();
        } else {
            concept.~unique_ptr();
        }
    }

    ~Function() {
        destroy();
    }

    void swap(Function<R(Args...)> &other) {
        Function temp(std::move(*this));
        *this = std::move(other);
        other = std::move(temp);
    }

    R operator()(Args ...args) {
        return get_concept()->invoke(args...);
    }

    R operator()(Args ...args) const {
        return get_concept()->invoke(args...);
    }

    Function &operator=(Function other) {
        swap(other);
        return *this;
    }

    Function &operator=(Function &&other) noexcept {
        destroy();
        swap(other);
    }

    struct Concept {
        virtual R invoke(Args... args) = 0;
        virtual R invoke(Args... args) const = 0;
        virtual void placement_copy_small(void *ptr) const = 0;
        virtual void placement_copy_big(void *ptr) const = 0;
        virtual void placement_move(void *ptr) = 0;
    };

    template<typename F>
    struct Model : Concept {
        explicit Model(F f) : f(std::move(f)) {}

        R invoke(Args... args) {
            return f(std::forward(args)...);
        }

        R invoke(Args... args) const {
            return f(std::forward(args)...);
        }

        void placement_copy_small(void *ptr) const {
            new(ptr) Model<F>(f);
        }

        void placement_copy_big(void *ptr) const {
            new(ptr) std::unique_ptr<Concept>(new Model<F>(f));
        }

        void placement_move(void *ptr) {
            new(ptr) Model<F>(std::move(f));
        }

        mutable F f;
        // because f::operator() can be not const;
    };

    const Concept *get_concept() const {
        if (isSmall) {
            return reinterpret_cast<const Concept *>(buf);
        } else {
            return concept.get();
        }
    }

    Concept *get_concept() {
        if (isSmall) {
            return reinterpret_cast<Concept *>(buf);
        } else {
            return concept.get();
        }
    }

    union {
        std::unique_ptr<Concept> concept;
        char buf[BUF_SIZE];
    };
    bool isSmall;
};
#pragma once

#include <functional>
#include <memory>


template<typename>
class Function;


template<typename R, typename ...Args>
struct Function<R(Args...)> {

    static const int BUF_SIZE = 16;

    Function() noexcept : Function(nullptr) {}

    Function(std::nullptr_t) noexcept : concept() {
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

    Function(Function &&other) noexcept {
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

    ~Function() {
        destroy();
    }

    Function &operator=(Function other) {
        swap(other);
        return *this;
    }

    Function &operator=(Function &&other) noexcept {
        destroy();
        swap(other);
        return *this;
    }

    void swap(Function &other) noexcept {
        Function temp(std::move(*this));
        new (this) Function(std::move(other));
        new (&other) Function(std::move(temp));
    }

    operator bool() {
        return isSmall && concept;
    }

    R operator()(Args ...args) const {
        return get_concept()->invoke(args...);
    }

    void destroy() {
        if (isSmall) {
            get_concept()->~Concept();
        } else {
            concept.~unique_ptr();
        }
    }

    struct Concept {
        virtual R invoke(Args... args) const = 0;
        virtual void placement_copy_small(void *ptr) const = 0;
        virtual void placement_copy_big(void *ptr) const = 0;
        virtual void placement_move(void *ptr) noexcept = 0;
    };

    template<typename F>
    struct Model : Concept {
        explicit Model(F f) : f(std::move(f)) {}

        R invoke(Args... args) const {
            return f(std::forward<Args>(args)...);
        }

        void placement_copy_small(void *ptr) const {
            new(ptr) Model<F>(f);
        }

        void placement_copy_big(void *ptr) const {
            new(ptr) std::unique_ptr<Concept>(new Model<F>(f));
        }

        void placement_move(void *ptr) noexcept {
            new(ptr) Model<F>(std::move(f));
        }

        F f;
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
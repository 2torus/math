#include <gtest/gtest.h>
#include <stan/math/rev/core.hpp>

#include <thread>

TEST(thread_stack_instance, initialize) {
  using stan::math::ChainableStack;

  // the main thread must be initialized by the time this code is
  // reached. This will actually segfault if this is not the case.
  // The pointer to the reference returned must evaluate to true.
  EXPECT_TRUE(&ChainableStack::instance());

  ChainableStack::AutodiffStackStorage& main_ad_stack
      = ChainableStack::instance();

  auto thread_tester = [&]() -> void {
    ChainableStack thread_instance;
    EXPECT_TRUE(&ChainableStack::instance());
    EXPECT_TRUE(&ChainableStack::instance()
#ifdef STAN_THREADS
                !=
#else
                ==
#endif
                &main_ad_stack);
  };
  std::thread other_work(thread_tester);

  other_work.join();
}

TEST(thread_stack_instance, child_instances) {
  using stan::math::ChainableStack;
  // place a var on the stack such that a fresh stack in another
  // thread will be different at initialization (if STAN_THREADS is
  // set)
  stan::math::var a = 1;

  ChainableStack::AutodiffStackStorage& main_ad_stack
      = ChainableStack::instance();

  auto thread_tester = [&]() -> void {
    ChainableStack thread_instance;
    EXPECT_TRUE(main_ad_stack.var_stack_.size()
#ifdef STAN_THREADS
                >
#else
                ==
#endif
                ChainableStack::instance().var_stack_.size());
  };

  std::thread other_work(thread_tester);

  other_work.join();
}

TEST(thread_stack_instance, persistence) {
  using stan::math::ChainableStack;

  ChainableStack::AutodiffStackStorage& main_ad_stack
      = ChainableStack::instance();

  {
    // this obviously must be true
    EXPECT_TRUE(&ChainableStack::instance() == &main_ad_stack);

    // and when constructing a new instance ...
    ChainableStack another_tape;

    // ... this must still be true ...
    EXPECT_TRUE(&ChainableStack::instance() == &main_ad_stack);
  }

  // ... and if we now let another_tape fade away we still expect the
  // main tape to stay around
  EXPECT_TRUE(&ChainableStack::instance() == &main_ad_stack);
}

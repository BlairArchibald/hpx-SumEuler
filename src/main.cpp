#include <iostream>

#include <hpx/hpx_init.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/iostreams.hpp>
#include <hpx/include/thread_executors.hpp>
#include <hpx/include/serialization.hpp>

#include "workqueue_component.hpp"

#include <vector>
#include <numeric>

#define UPPER 20000
#define LOWER 1
#define CHUNK_SIZE 100

namespace euler {
  // Don't have std::gcd so we write a simple one here
  auto gcd(unsigned a, unsigned b) -> unsigned {
    return (b == 0) ? a : gcd (b, a % b);
  }

  auto totient(unsigned n) -> unsigned {
    unsigned c = 0;
    for (unsigned i = 1; i <= n; i++) {
      if (gcd(n, i) == 1) {
        c++;
      }
    }
    return c;
  }

  auto sumTotient(unsigned lower, unsigned upper) -> unsigned {
    unsigned t = 0;
    for (auto i = lower; i <= upper; i++) {
      t += totient(i);
    }
    return t;
  }
}

unsigned sumEulerSeq(unsigned lower, unsigned upper) {
  return euler::sumTotient(lower, upper);
}

void sumEuler(unsigned lower, unsigned upper, hpx::naming::id_type promise) {
  hpx::apply<hpx::lcos::base_lco_with_value<unsigned>::set_value_action>(promise, euler::sumTotient(lower, upper));
}
HPX_PLAIN_ACTION(sumEuler, sumEulerAction);

namespace scheduler {
  auto cancelScheduler() -> void;
  auto scheduler(hpx::naming::id_type workqueue) -> void;
}
HPX_PLAIN_ACTION(scheduler::cancelScheduler, cancelSchedulerAction)
HPX_PLAIN_ACTION(scheduler::scheduler, schedulerAction)

namespace scheduler {
  std::atomic<bool> running(true);
  auto cancelScheduler() -> void {
    running = false;
  }

  auto scheduler(hpx::naming::id_type workqueue) -> void {
    auto threads = hpx::get_os_thread_count() == 1 ? 1 : hpx::get_os_thread_count() - 1;
    hpx::threads::executors::local_queue_executor scheduler(threads);

    // Debugging
    std::cout << "Running scheduler with: " << threads << " scheduler threads" << std::endl;

    while (running) {
      auto pending = scheduler.num_pending_closures();
      if (pending < threads) {
        auto task = hpx::async<workstealing::component::workqueue::steal_action>(workqueue).get();
        if (task) {
          scheduler.add(task);
        } else {
          hpx::this_thread::suspend();
        }
      } else {
        hpx::this_thread::suspend();
      }
    }
  }
}

int hpx_main(boost::program_options::variables_map & opts) {
  if (opts["sequential"].as<bool>()) {
    auto sum = sumEulerSeq(LOWER, UPPER);
    std::cout << "Final sum was: " << sum << std::endl;
    return hpx::finalize();
  } else {
    // Create workqueue and scheduler
    auto workqueue = hpx::new_<workstealing::component::workqueue>(hpx::find_here()).get();
    hpx::apply<schedulerAction>(hpx::find_here(), workqueue);

    // Create sumeuler work items - I think there is potentially a bounds issue here?
    auto numPromises = std::ceil((UPPER - LOWER) / CHUNK_SIZE);
    std::vector<hpx::lcos::promise<unsigned>> promises(numPromises + 1);
    std::vector<hpx::lcos::future<unsigned>> futures;
    auto p = 0;
    for (int i = LOWER; i < UPPER; i += CHUNK_SIZE) {
      auto f = promises[p].get_future();
      auto id = promises[p].get_id();
      futures.push_back(std::move(f));

      hpx::util::function<void()> task = hpx::util::bind(sumEulerAction(), hpx::find_here(), i, i + CHUNK_SIZE - 1, id);
      hpx::async<workstealing::component::workqueue::addWork_action>(workqueue, task).get();

      p++;
    }

    hpx::wait_all(futures);

    auto sum = 0;
    for (auto & f : futures) {
      sum += f.get();
    }

    std::cout << "Final sum was: " << sum << std::endl;

    scheduler::cancelScheduler();

    return hpx::finalize();
  }
}

int main (int argc, char* argv[]) {
  boost::program_options::options_description
    desc_commandline("Usage: " HPX_APPLICATION_STRING " [options]");

  desc_commandline.add_options()
    ( "sequential,s", boost::program_options::bool_switch()->default_value(false),
      "Run sequentially"
    );

    return hpx::init(desc_commandline, argc, argv);
}
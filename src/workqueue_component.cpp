#include "workqueue_component.hpp"
#include <hpx/include/iostreams.hpp>
#include <hpx/include/lcos.hpp>

namespace workstealing { namespace component {
  using funcType = hpx::util::function<void()>;

  funcType workqueue::steal() {
      if (!tasks.empty()) {
        auto task = tasks.front();
        tasks.pop();
        return task;
      }
      return nullptr;
    }

    void workqueue::addWork(funcType task) {
      tasks.push(task);
    }
}}
HPX_REGISTER_COMPONENT_MODULE();

typedef hpx::components::component<workstealing::component::workqueue> workqueue_type;

HPX_REGISTER_COMPONENT(workqueue_type, workqueue);

HPX_REGISTER_ACTION(workstealing::component::workqueue::steal_action, workqueue_steal_action);
HPX_REGISTER_ACTION(workstealing::component::workqueue::addWork_action, workqueue_addWork_action);

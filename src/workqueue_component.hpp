#ifndef HELLO_WORLD_COMPONENT_HPP
#define HELLO_WORLD_COMPONENT_HPP

#include <hpx/hpx.hpp>
#include <hpx/include/actions.hpp>
#include <hpx/include/lcos.hpp>
#include <hpx/include/components.hpp>
#include <hpx/include/serialization.hpp>
#include <hpx/include/iostreams.hpp>

#include <boost/variant.hpp>

#include <utility>
#include <iostream>
#include <queue>

namespace workstealing { namespace component {
  class workqueue : public hpx::components::locking_hook<
    hpx::components::component_base<workqueue>
    > {
    private:
      using funcType = hpx::util::function<void()>;
      std::queue<funcType> tasks;

    public:
      funcType steal();
      HPX_DEFINE_COMPONENT_ACTION(workqueue, steal);
      void addWork(funcType task);
      HPX_DEFINE_COMPONENT_ACTION(workqueue, addWork);
    };
}}

HPX_REGISTER_ACTION_DECLARATION(workstealing::component::workqueue::steal_action, workqueue_steal_action);
HPX_REGISTER_ACTION_DECLARATION(workstealing::component::workqueue::addWork_action, workqueue_addWork_action);
#endif


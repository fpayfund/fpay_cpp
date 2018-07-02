#ifndef AUTODONE_H_
#define AUTODONE_H_

namespace core{

/**
 * @class AutoDone
 *
 * 提供自动 scoped 操作保障，保证操作对象的 cleanup 操作在 AutoDone 对象生命周期结束时能被调用。
 */
template< class t_task >
class AutoDone
{
public:
  typedef t_task task_type;

public:
  /**
   * 构造方法。
   * @param task 操作对象。
   * @param todo 需要执行的操作，如果不等于 NULL，将立刻被调用。
   * @param done 需要执行的 cleanup 操作，如果不等于 NULL，将会被在 AutoDone 析构函数里调用。
   */
  AutoDone
  (
    task_type& task,
    void ( task_type::*todo )(),
    void ( task_type::*done )()
  ) : task_( task ), do_( todo ), done_( done )
  {
    if ( NULL != do_ )
    {
      ( task_.*do_ )();
    }
  }

  /**
   * 析构方法。
   */
  ~AutoDone()
  {
    if ( NULL != done_ )
    {
      ( task_.*done_ )();
    }
  }
  
private:
  AutoDone();
  AutoDone( const AutoDone& );
  AutoDone& operator=( const AutoDone& );

private:
  task_type& task_;
  void ( task_type::*do_ )();
  void ( task_type::*done_ )();
};

}

#endif//


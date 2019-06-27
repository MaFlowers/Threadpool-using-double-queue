# Threadpool-using-double-queue
There's one version for threadpool in linux platform, which is a kind of task allocation.
Explain:
This thread pool is like a factory.And the factory usually consists of the following roles: supervisor, assistant, employee and customer.
Customers provide tasks, assistant receives and presents tasks, supervisor assigns task, and employees perform tasks.
This thread pool's working principle as follows:
                           new     To perform
                          -----      -----
 -----------     add new |  t  |    |  t  |   
| customers | ---------> |  a  |    |  a  |  get tasks  ------------
 -----------      tasks  |  s  |    |  s  | ---------->| supervisor |
                         |  k  |    |  k  |             -------------       
                          -----      -----                   |
                            |          ^                     | assigns tasks
                            |tasks swap|                     ↓
                             ----------                -------------
                                 |                    |  employees  |  
				 |                     -------------
                           -------------               perform tasks
                          |  assistant  |
                           -------------													   

#include"connectionPool.h"
#include"public.h"



// 线程安全的懒汉单例模式
ConnectionPool *ConnectionPool::instance(){
    static ConnectionPool pool;
    return &pool;
}

ConnectionPool::ConnectionPool() : _connectionCnt(0),_isRunning(true){
    // 加载配置项
	if(!loadConfigFile()){
		return;
	}
	// 创建初始数量的连接
	for(int i=0;i<_initSize;i++){
		// 创建连接
		Connection* p=new Connection();
		// 连接到服务器
		p->connect(_ip,_port,_username,_password,_dbname);
		_connectionQueue.push(p);
		// 刷新一下开始空闲的起始时间
		p->refreshAliveTime();
		_connectionCnt++;
	}

	// 启动一个新的线程，作为生产者生产连接
	thread produceConnTaskThread(bind(&ConnectionPool::produceConnectionTask,this));
	produceConnTaskThread.detach();

	thread scannerConnTaskThread(bind(&ConnectionPool::scannerConnectionTask,this));
	scannerConnTaskThread.detach();
}

bool ConnectionPool::loadConfigFile(){
    FILE* pf=fopen("mysql.cnf","r");
    if(pf==nullptr){
        LOG("mysql.cnf file is not exist!");
        return false;
    }
    while(!feof(pf)){
        char line[1024]={0};
        fgets(line,1024,pf);
        string str=line;
        int idx=str.find('=',0);
        // 未找到
        if(idx==-1){
            continue;
        }

        int endidx=str.find('\n',idx);
        string key=str.substr(0,idx);
        string value=str.substr(idx+1,endidx-idx-1);

        if (key == "ip"){
			_ip = value;
		}
		else if (key == "port"){
			_port = atoi(value.c_str());
		}
		else if (key == "username"){
			_username = value;
		}
		else if (key == "password"){
			_password = value;
		}
		else if (key == "dbname"){
			_dbname = value;
		}
		else if (key == "initSize"){
			_initSize = atoi(value.c_str());
		}
		else if (key == "maxSize"){
			_maxSize = atoi(value.c_str());
		}
		else if (key == "maxIdleTime"){
			_maxIdleTime = atoi(value.c_str());
		}
		else if (key == "connectionTimeOut"){
			_connectionTimeOut = atoi(value.c_str());
		}
    }
    return true;   
}
shared_ptr<Connection> ConnectionPool::getConnection(){
	unique_lock<mutex> lock(_queueMutex);
	while(_connectionQueue.empty()){
		// 如果超过等待时间，则不在阻塞等待 
		if(cv_status::timeout==cv.wait_for(lock,chrono::microseconds(_connectionTimeOut))){
			// 队列仍然为空
			if(_connectionQueue.empty()){
				LOG("获取空闲连接超时");
				return nullptr;
			}
		}
	}
	/*
		shared_ptr在析构时会将connection释放，但是我们需要的不是释放，而是将资源归还，所以重写智能指针的析构方法
	*/
	shared_ptr<Connection> sp(_connectionQueue.front(),[&](Connection* pcon){
		// 确保线程安全
		unique_lock<mutex> lock(_queueMutex);
		_connectionQueue.push(pcon);
		// 刷新一下开始空闲的起始时间
		pcon->refreshAliveTime();
		cv.notify_all();
	});
	// 能运行到这儿说明此刻是上锁的，所以是线程安全的
	_connectionQueue.pop();

	// 通知其他消费者或生产者
	cv.notify_all();
    return sp;
}


//运行在独立的线程中，专门负责生成新连接
void ConnectionPool::produceConnectionTask(){
	for(;;){
		while(_isRunning){
			unique_lock<mutex> lock(_queueMutex);
			while(!_connectionQueue.empty()){
				// 队列不空，此处生产线程进入等待状态
				cv.wait(lock);
			}
			// 只有在连接数小于最大连接数时才创建
			if(_connectionCnt<_maxSize){
				// 创建连接
				Connection* p=new Connection();
				// 连接到服务器
				p->connect(_ip,_port,_username,_password,_dbname);
				// 刷新一下开始空闲的起始时间
				p->refreshAliveTime();
				_connectionQueue.push(p);
				_connectionCnt++;
			}
		}
		// 通知消费者线程
		cv.notify_all();
	}
}

// 启动一个定时线程，扫描多余的空闲连接，专门用于销毁空闲时间过长的连接
void ConnectionPool::scannerConnectionTask(){
	for(;;){
		while(_isRunning){
			// 模拟定时效果
			this_thread::sleep_for(chrono::seconds(_maxIdleTime));
			// 扫描整个队列，释放多余的连接
			unique_lock<mutex> lock(_queueMutex);
			while(_connectionCnt>_initSize){
				// 因为是按顺序入队的，所以队头的空闲存活时间是最长的
				Connection* p=_connectionQueue.front();
				if(p->getAliveTime()>=(_maxIdleTime*1000)){
					_connectionQueue.pop();
					_connectionCnt--;
					// 调用connection的析构函数，释放连接
					delete p;
				}
				// 最长的空闲存活时间都没有超过_maxIdleTime，则跳过
				break;
			}
		}
	}
}

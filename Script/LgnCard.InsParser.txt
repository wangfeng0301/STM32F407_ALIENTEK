// 如果其他环境运行，增加以下一行代码
//var Ins = new ActiveXObject("LgnCard.InsParser");
/* 在IE中使用
  var mgr = new ActiveXObject("LgnCard.LgnManager");
  var Ins = mgr.CreateInstance("LgnCard.InsParser");
*/

/* 功能：执行指令（IC指令和自定义指令）
   参数：cmd-指令+数据；参照原脚本格式
         cond-条件；参照原脚本格式	
   说明：直接调用COM组件执行指令，该封装函数便于调试输出。
*/
function Ins_ExecuteCmd(cmd, cond)
{
	var rv = true;
	try{
		Ins.ExecuteSingle(cmd, cond);
	}
	catch(e){
		rv = false;
	}

	var info = "名称: " + Ins.GetInsDesc("");
	info = info + "   指令: " + Ins.GetIns();
	info = info + "   状态: " + Ins.GetSW();
	info = info + "   返回: " + Ins.GetRet();
	info = info + "   描述: " + Ins.GetErrMsg();
	Debug.writeln(info);
	if (!rv){
		throw e;	
	}
	return Ins.GetRet();
}


// LgnCard.InsParser 的函数说明
function Ins_GetRet()
{
	return Ins.GetRet();
}

function Ins_GetSW()
{
	return Ins.GetSW();
}
function Ins_GetSW1()
{
	return Ins.GetSW().substr(0,2);
}
function Ins_GetSW2()
{
	return Ins.GetSW().substr(2,2);
}

// ------------------------------变量管理--------------------------------
// 设置变量
function Ins_PutInsVar(key, value)
{
	Ins.InsVar(key) = value;
}

// 获取变量
function Ins_GetInsVar(key)
{
	return Ins.InsVar(key);
}

// ------------------------------参数管理--------------------------------
// 设置参数
function Ins_PutInsParam(param, data)
{
	Ins.InsParam(param) = data;
}

// 获取参数
function Ins_GetInsParam(param)
{
	return Ins.InsParam(param);
}

//
function Ins_Save(flags) // 1-obj, 2-state
{
	Ins.Save(flags);
}
function Ins_Load(flags)
{
	Ins.Load(flags);
}

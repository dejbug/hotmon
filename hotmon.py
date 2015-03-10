import ctypes as c
import ctypes.wintypes as w
import vkdefs
import threading


INFINITE = 0xFFFFFFFF


E_OK				= 0
E_NULL_ARG			= 1
E_OUT_OF_MEMORY		= 2
E_EVENT_CREATE		= 3
E_EVENT_USAGE		= 4
E_NOT_RUNNING		= 5
E_STILL_RUNNING		= 6
E_ALREADY_RUNNING	= 7
E_START_TIMEOUT		= 8
E_STOP_TIMEOUT		= 9
E_ATOM_CREATE		= 10
E_HOTKEY_NOT_INIT	= 11
E_TIMEOUT			= 12


E_LABELS = [
	"E_OK",
	"E_NULL_ARG",
	"E_OUT_OF_MEMORY",
	"E_EVENT_CREATE",
	"E_EVENT_USAGE",
	"E_NOT_RUNNING",
	"E_STILL_RUNNING",
	"E_ALREADY_RUNNING",
	"E_STARTUP_TIMEOUT",
	"E_ATOM_CREATE",
	"E_HOTKEY_NOT_INIT",
	"E_TIMEOUT",
]


class Error(Exception): pass

class HotmonError(Error):
	def __init__(self, code):
		self.code = code
		Error.__init__(self, "[%d] (%s)" % (self.code, E_LABELS[self.code]))


dll = c.CDLL("hotmon.dll")


CALLBACK = c.CFUNCTYPE(None, w.LPVOID)


class HOTMON(c.Structure):
	_fields_ = [
		("hwnd", w.HWND),
		("event", w.HANDLE),
		("tid", w.DWORD),
	]
PHOTMON = c.POINTER(HOTMON)
PPHOTMON = c.POINTER(PHOTMON)


class HOTKEY(c.Structure):
	_fields_ = [
		("id", w.ATOM),
		("vk", w.UINT),
		("mod", w.UINT),
		("callback", CALLBACK),
		("param", w.LPARAM),
	]
PHOTKEY = c.POINTER(HOTKEY)


def errcheck(result, func, args):
	if E_OK != result:
		raise HotmonError(result)
	return args


#hmCreate_proto = c.CFUNCTYPE(c.c_int, c.c_int)
#hmCreate_paraf = (1, "hm", 0), 
#hmCreate = hmCreate_proto(("hmCreate", dll), hmCreate_paraf)
#hmCreate.errcheck = errcheck


class API(object):
	def __init__(self, *signature):
		"""The arguments to this constructor are caught in 'signature',
		which is a sequence of 3-tuples (arg_name, arg_type, arg_default).
		In this version, all arguments are supposed to be input arguments
		and will have the default value of 0."""
		
		arg_types = (c.c_int, ) + tuple(item[1] for item in signature)
		
		self.proto = c.CFUNCTYPE(*arg_types)
		self.paraf = tuple((1, item[0], item[2]) for item in signature)
		
	def __call__(self, func):
		fwrap = self.proto((func.__name__, dll), self.paraf)
		fwrap.errcheck = errcheck
		return fwrap


@API(("hm", PPHOTMON, None))
def hmCreate(): pass

@API(("hm", PPHOTMON, None))
def hmDelete(): pass

@API(("hm", PPHOTMON, None), ("msec", w.DWORD, 3000))
def hmStart(): pass

@API(("hm", PPHOTMON, None), ("msec", w.DWORD, 3000))
def hmStop(): pass

@API(("hm", PPHOTMON, None), ("hk", PHOTKEY, None), ("vk", w.UINT, 0), ("mod", w.UINT, 0), ("callback", CALLBACK, None), ("param", w.LPVOID, None))
def hmCreateHotkey(): pass

@API(("hm", PPHOTMON, None), ("hk", PHOTKEY, None))
def hmAddHotkey(): pass

@API(("hm", PPHOTMON, None), ("hk", PHOTKEY, None))
def hmRegister(): pass

@API(("hm", PPHOTMON, None), ("hk", PHOTKEY, None))
def hmUnregister(): pass


if "__main__" == __name__:
	event = threading.Event()
	
	def quit_callback(param):
		event.set()
		
	def action_callback(param):
		print "f1"
		
	# these are important! otherwise they'll get garbage collected!
	quit_callback_ref = CALLBACK(quit_callback)
	action_callback_ref = CALLBACK(action_callback)
	
	hm = PHOTMON()
	hmCreate(hm)
	hmStart(hm)
	
	quit_hk = HOTKEY()
	hmCreateHotkey(hm, quit_hk, vkdefs.VK_ESCAPE, 0, quit_callback_ref)
	
	action_hk = HOTKEY()
	hmCreateHotkey(hm, action_hk, vkdefs.VK_F1, vkdefs.MOD_CONTROL|vkdefs.MOD_SHIFT, action_callback_ref)
	
	hmAddHotkey(hm, quit_hk)
	hmAddHotkey(hm, action_hk)
	
	print "(to quit the app, hit the ESC key)"
	print "(try pressing CTRL+SHIFT+F1)"
	event.wait()
	
	hmStop(hm)
	hmDelete(hm)
	

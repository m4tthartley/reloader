
#include <fileapi.h>
#include <stdatomic.h>
#include <stdio.h>
#include <stdatomic.h>

#include <core.h>

typedef struct {
	string dll_filename;
	string dll_path;
	string dir_path;
} state_t;

typedef void* (*start_proc)(void);
typedef void (*frame_proc)(void* param);

b32 do_reload = FALSE;
HANDLE lib;
start_proc start;
frame_proc frame;

void core_error(char* err, ...) {
	char str[1024];
	va_list va;
	va_start(va, err);
	vsnprintf(str, 1024, err, va);
	printf("%s\n", str);
	va_end(va);
}

char _win32_error_buffer[1024];
char* win32_error() {
	DWORD error = GetLastError();
	FormatMessage(
		FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		error,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPWSTR)_win32_error_buffer,
		sizeof(_win32_error_buffer),
		NULL);
	return _win32_error_buffer;
}

FILE_NOTIFY_INFORMATION file_changes[16];

void completion_routine(DWORD dwErrorCode, DWORD dwNumberOfBytesTransfered, LPOVERLAPPED lpOverlapped) {
	printf("completion routine \n");
	FILE_NOTIFY_INFORMATION* file = file_changes;
	while(file) {
		printf("%s \n", file->FileName);
		// if()

		file = NULL;
		if(file->NextEntryOffset) {
			file = (u8*)file + file->NextEntryOffset;
		}
	}
}

DWORD dir_listen_thread(void* lp) {
	state_t* state = lp;

	HANDLE file_handle = CreateFileA(
		state->dir_path,
		FILE_LIST_DIRECTORY,
		FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS|FILE_FLAG_OVERLAPPED,
		NULL);
	if(!file_handle == INVALID_HANDLE_VALUE) {
		printf("\033[91mCreateFileA failed\033[0m \n");
	}

	long bytes;
	int read;

	OVERLAPPED overlapped = {0};
	overlapped.hEvent = CreateEventA(NULL, FALSE, FALSE, NULL);

	if(!ReadDirectoryChangesW(
		file_handle,
		file_changes,
		sizeof(file_changes),
		TRUE,
		FILE_NOTIFY_CHANGE_LAST_WRITE,
		&bytes,
		&overlapped,
		NULL)) {
		core_error("ReadDirectoryChanges: %s", win32_error());
	}

	for(;;) {
		DWORD wait = WaitForSingleObject(overlapped.hEvent, INFINITE);
		
		if(wait == WAIT_OBJECT_0) {
			printf("wait object 0 \n");
		} else {
			printf("wait error \n");
		}

		FILE_NOTIFY_INFORMATION* file = file_changes;
		while(file) {
			// TODO maybe this can be a core string function
			char filename[MAX_PATH+1] = {0};
			for(int i=0; i<file->FileNameLength/sizeof(*file->FileName); ++i) {
				filename[i] = file->FileName[i];
			}
			printf("%s \n", filename);
			if(s_compare(filename, state->dll_filename)) {
				printf("RELOAD \n");
				atomic_swap32(&do_reload, TRUE);
			}

			if(file->NextEntryOffset) {
				file = (u8*)file + file->NextEntryOffset;
			} else {
				file = NULL;
			}
		}

		zeroMemory(file_changes, sizeof(file_changes));

		if(!ReadDirectoryChangesW(
			file_handle,
			file_changes,
			sizeof(file_changes),
			TRUE,
			FILE_NOTIFY_CHANGE_LAST_WRITE,
			&bytes,
			&overlapped,
			NULL)) {
			core_error("ReadDirectoryChanges: %s", win32_error());
		}
	}

	return 0;
}

void reload(state_t* state) {
	FreeLibrary(lib);
	string copy_dll = s_format("%scopy_%s", state->dir_path, state->dll_filename);
	int copy = CopyFile(state->dll_path, copy_dll, 0);
	lib = LoadLibraryA(copy_dll);
	start = (start_proc*)GetProcAddress(lib, "start");
	frame = (frame_proc*)GetProcAddress(lib, "frame");
}

int main(int argc, char** argv) {
	printf("Reloader \n");

	if(argc < 2) {
		printf("Arg 1 should be a directory to watch \n");
		exit(1);
	}

	state_t state = {0};

	u8 strBuffer[PAGE_SIZE];
	string_pool spool;
	s_create_pool(&spool, strBuffer, sizeof(strBuffer));
	s_pool(&spool);

	HANDLE file = CreateFileA(argv[1], GENERIC_READ, FILE_SHARE_DELETE|FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if(file == INVALID_HANDLE_VALUE) {
		printf("file %s could not be found \n", argv[1]);
		exit(1);
	}

	// char dir_buffer[MAX_PATH] = {0};
	// PathCchRemoveFileSpec(dir_buffer, MAX_PATH);
	// printf("dir %s \n", dir_buffer);
	state.dll_path = s_copy(argv[1]);
	char* c = argv[1] + s_len(argv[1]);
	while(*c != '/' && c >= argv[1]) {
		// *c = 0;
		--c;
	}
	state.dll_filename = s_copy(c+1);
	c[1] = 0;

	// string dll_path = "./test.dll";
	state.dir_path = argv[1];
	if(!s_len(state.dir_path)) {
		state.dir_path = "./";
	}
	// printf(dll_path);

	CreateThread(0, 0, dir_listen_thread, &state, 0, 0);

	reload(&state);

	PULONG low;
	PULONG high;
	GetCurrentThreadStackLimits(&low, &high);
	register void *sp asm ("sp");

	void* user_param = start();
	for(;;) {
		// atomic_compare_exchange_weak(&do_reload, TRUE, FALSE);
		if(atomic_compare_swap32(&do_reload, TRUE, FALSE)) {
			reload(&state);
		}
		frame(user_param);
	}
	// for(;;) {
	// 	Sleep(1);
	// }

	return 0;
}

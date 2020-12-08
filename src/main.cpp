#include "connection.h"
#include "memory.h"
#include "polymer.h"

#include <cassert>
#include <cstdio>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#define WIN32_LEAN_AND_MEAN
#include <WS2tcpip.h>
#include <Windows.h>

namespace polymer {

int run() {
  constexpr size_t perm_size = megabytes(32);
  u8* perm_memory = (u8*)VirtualAlloc(NULL, perm_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  constexpr size_t trans_size = megabytes(32);
  u8* trans_memory = (u8*)VirtualAlloc(NULL, trans_size, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

  MemoryArena perm_arena(perm_memory, perm_size);
  MemoryArena trans_arena(trans_memory, trans_size);

  printf("Polymer\n");

  Connection* connection = memory_arena_construct_type(&perm_arena, Connection, perm_arena);

  ConnectResult connect_result = connection->Connect("127.0.0.1", 25566);

  switch (connect_result) {
  case ConnectResult::ErrorSocket: {
    fprintf(stderr, "Failed to create socket\n");
    return 1;
  }
  case ConnectResult::ErrorAddrInfo: {
    fprintf(stderr, "Failed to get address info\n");
    return 1;
  }
  case ConnectResult::ErrorConnect: {
    fprintf(stderr, "Failed to connect\n");
    return 1;
  }
  default:
    break;
  }

  printf("Connected to server.\n");

  connection->SetBlocking(false);

  while (connection->connected) {
    int bytes_recv = recv(connection->fd, (char*)connection->buffer.data + connection->buffer.write_offset,
                          (u32)connection->buffer.GetFreeSize(), 0);

    if (bytes_recv == 0) {
      fprintf(stderr, "Bytes recv zero\n");
      connection->connected = false;
      break;
    } else if (bytes_recv < 0) {
      int err = WSAGetLastError();

      if (err == WSAEWOULDBLOCK) {
        continue;
      }

      fprintf(stderr, "Error: %d\n", err);
      connection->Disconnect();
      break;
    }

    connection->buffer.write_offset = (connection->buffer.write_offset + bytes_recv) % connection->buffer.size;

    printf("%.*s\n", bytes_recv, connection->buffer.data + connection->buffer.read_offset);

    connection->buffer.read_offset = (connection->buffer.read_offset + bytes_recv) % connection->buffer.size;
  }

  return 0;
}

} // namespace polymer

int main(int argc, char* argv[]) {
  return polymer::run();
}

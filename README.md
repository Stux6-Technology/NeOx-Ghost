<p align="center">
  <img src="NeOx%20Ghost.png" alt="NeOx Ghost" width="100%">
</p>

## What is NeOx Ghost Project
**NeOx-Ghost** is an experimental, cyber defense-focused multi-server microkernel operating system project developed under Stux6 Technology Labs. It is built upon a heavily optimized, streamlined, and stripped-down **GNU Hurd** and Mach microkernel baseline.

The project introduces a radical architectural shift in system security by completely rethinking how network communication interacts with the lowest levels of an operating system.

## Core Architectural Concepts
  - **Decoupling the Network Stack:** In traditional monolithic kernels, a vulnerability in a network driver or packet parser can compromise the entire system at Ring 0. NeOx-Ghost aims to isolate these components into separate user-space servers (translators) to ensure that a compromise or crash in the network layer cannot bring down the core system infrastructure.
  - **Ring 0 / Ring -1 Cryptographic Routing:** Instead of running traditional, high-latency user-space proxies, the project focuses on embedding a highly optimized, low-level routing fabric directly at the hardware abstraction layer.
  - **Native i2p Integration (The "Evil Twin" Engine):** The project integrates the architectural principles of `i2p` (**Invisible Internet Project**) straight into the core infrastructure. All inbound and outbound Layer 3/4 (TCP/UDP) network traffic is forcefully routed through an automated encryption layer, making network operations hardware-anonymized by default.
  - **Parallel Packet Splitting (Parallel Garlic Routing):** To combat the inherent latency issues of anonymous routing networks, **NeOx-Ghost** is designed to automatically split outbound data packets into 10 or more distinct streams, routing them through parallel channels across multiple servers before reconstructing them dynamically.

## What is the purpose of the NeOx Ghost Project?
The ultimate aim of the **NeOx-Ghost** project is to establish a radical cyber defence operating system architecture that completely eliminates the security vulnerabilities inherent in the monolithic (single-piece) network architecture of traditional operating systems, whilst providing anonymity and immunity to cyber-attacks at the hardware level.

To achieve this aim, the project aims to fulfil the following three key operational objectives:
### A) User-Space Driver Isolation
Unlike traditional systems, the kernel does not panic when the network card receives data. The driver operates as an isolated ‘Translator’ server in user space. If the driver crashes, the Mach IPC (`Inter-Process Communication`) mechanism detects the situation and immediately restarts the driver without causing the system to crash.
```C
#include <stdio.h>
#include <stdbool.h>

// Sürücünün durumunu simüle eden Mach IPC yapısı
typedef enum { DRIVER_HEALTHY, DRIVER_CRASHED } driver_state_t;

// Kullanıcı alanında (User-Space) çalışan soyutlanmış ağ sürücüsü
struct NetworkDriverServer {
    int port_id;
    driver_state_t state;
};

// NeOx Çekirdek Seviyesi Gözetleme Mekanizması (Mach IPC Monitor)
void neox_ipc_monitor(struct NetworkDriverServer *driver) {
    if (driver->state == DRIVER_CRASHED) {
        printf("[NeOx-Kernel] UYARI: Kullanıcı alanı ağ sürücüsü (Port: %d) çöktü!\n", driver->port_id);
        printf("[NeOx-Kernel] Ring 0 güvende. Çekirdek kararlılığı bozulmadı.\n");
        
        // Sürücüyü izole et ve anında yeniden ayağa kaldır (Respawn)
        driver->state = DRIVER_HEALTHY;
        printf("[NeOx-Kernel] Sürücü sunucusu otomatik olarak yeniden başlatıldı (IPC re-bound).\n");
    } else {
        printf("[NeOx-Kernel] Ağ sürücüsü izole alanda sorunsuz çalışıyor.\n");
    }
}

int main() {
    struct NetworkDriverServer eth0 = { .port_id = 1045, .state = DRIVER_CRASHED };
    // Çekirdek çökmesi yaşanmaz, sadece servis izole edilip tetiklenir
    neox_ipc_monitor(&eth0);
    return 0;
}
```

### B) Ring 0 Mandatory Encryption
Whatever the application layer does, data arriving at the network socket cannot be sent to the network card in its raw form. As soon as the packet reaches the lower abstraction layer (`Ring 0` / `Netfilter-like layer`), it must pass through a mandatory cryptographic tunnelling function.
```C
#include <stdio.h>
#include <string.h>

#define PACKET_SIZE 64

// Uygulama katmanından gelen ham TCP/UDP paketi
typedef struct {
    char data[PACKET_SIZE];
    bool is_anonymized;
} network_packet_t;

// Ring 0 düzeyinde zorunlu kriptografik yönlendirme katmanı
void neox_ring0_mandatory_crypto(network_packet_t *packet) {
    if (!packet->is_anonymized) {
        printf("[Ring 0] Ham veri tespit edildi: '%s'\n", packet->data);
        
        // Donanımsal seviyede zorunlu i2p/Ghost şifreleme simülasyonu (XOR / AES-GCM placeholder)
        for(int i = 0; i < strlen(packet->data); i++) {
            packet->data[i] ^= 0x5A; // Stux6 özel şifreleme maskesi
        }
        
        packet->is_anonymized = true;
        printf("[Ring 0] Paket ağ kartına (NIC) gitmeden önce zorunlu olarak şifrelendi (Hayalet Ağ).\n");
    }
}

int main() {
    network_packet_t user_packet = { .data = "SECRET_PAYLOAD_DATA", .is_anonymized = false };
    neox_ring0_mandatory_crypto(&user_packet);
    return 0;
}
```

### C) Parallel Garlic Routing
Gecikmeyi engellemek için giden tek bir paket donanım düzeyinde eşzamanlı olarak `N` parçaya bölünür ve her parça paralel olarak farklı i2p sanal tünellerine (`nxghost0..X`) dağıtılır.
```C
#include <stdio.h>
#include <stdlib.h>

#define TUNNEL_COUNT 4

// Paralel kanallara dağıtılacak paket parçacığı
typedef struct {
    int tunnel_id;
    char chunk_data;
} garlic_chunk_t;

// Paralel Paket Parçalama Motoru
void neox_parallel_garlic_route(const char *raw_data, int length) {
    printf("[Garlic Engine] Orijinal Veri: %s\n", raw_data);
    printf("[Garlic Engine] Veri %d paralel tünel üzerinden parçalanarak gönderiliyor...\n", TUNNEL_COUNT);

    garlic_chunk_t *dispatch_queue = malloc(sizeof(garlic_chunk_t) * length);

    // Eşzamanlı (Parallel) tünellere veriyi saçma algoritması
    for (int i = 0; i < length; i++) {
        dispatch_queue[i].tunnel_id = (i % TUNNEL_COUNT) + 1;
        dispatch_queue[i].chunk_data = raw_data[i];
        
        printf(" -> [Tünel #%d] '%c' karakteri paralel hattan gönderildi.\n", 
               dispatch_queue[i].tunnel_id, 
               dispatch_queue[i].chunk_data);
    }

    printf("[Garlic Engine] Tüm paralel kanallar eşzamanlı (multi-threaded) olarak tüketildi.\n");
    free(dispatch_queue);
}

int main() {
    const char *payload = "STUX6_CORE";
    neox_parallel_garlic_route(payload, 10);
    return 0;
}
```





## ⚖️ License
This project is protected under the **STUX6 GENERAL PRIVATE PROJECT LICENSE (SGPPL-v1.0)**. 
For full terms and conditions, please refer to the [LICENSE](LICENSE) file.

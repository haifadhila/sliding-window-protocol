## Sliding Window Protocol
**Tugas Besar I IF3130 Jaringan Komputer**
**Lossless Data Transfer**

Anggota:
 - Yuly Haruka Berliana Gunawan / 13516031
 - Haifa Fadhila Ilma / 13516076
 - Nadija Herdwina Putri Soerojo / 13516130
	(K-01)

### Petunjuk Penggunaan Program
Pertama, lakukan *Makefile*
```
$ make
```
Cara menjalankan program setelah di-*Makefile*:
1.	Pertama, jalankan Receiver, eksekusi pada command prompt/ terminal:
	```
	$ ./recvfile <filename> <windowsize> <buffersize> <port>
	```
	Contoh:
	```
	$ ./recvfile "hello.txt" 10 1000 2334
	```
	Contoh di atas akan menjalankan program untuk menerima file dengan *hello.txt* sebagai file yang diterima, ukuran window sebesar 10 untuk Sliding Window Protocol, ukuran buffer 1000 (dapat memuat 1000 frame) dan port 2334.

2. Kedua, jalankan Sender, eksekusi pada command prompt/ terminal:
	```
	$ ./sendfile <filename> <windowsize> <buffersize> <destination_ip> <destination_port> "
	```
	Contoh:
	```
	$ ./sendfile "test.txt" 10 1000 127.0.0.1 2334
	```
	Contoh di atas akan menjalankan program untuk mengirim file dengan *test.txt* sebagai file yang akan dikirim, ukuran window sebesar 10 untuk Sliding Window Protocol, ukuran buffer 1000 (dapat memuat 1000 frame), IP Address 127.0.0.1 dan port 2334.


### Cara Kerja Program
Sliding Window Protocol merupakan salah satu metode pengiriman file yang dipartisi menjadi *packets/frames*. Metode ini mendukung pengiriman lebih dari satu *packets/frames* dalam satu waktu.

Program yang kami buat (C++) merupakan implementasi dari cara kerja Sliding Window Protocol dengan Selective Repeat Automatic Repeat Request untuk transmisi data menggunakan UDP Socket.

Berikut cara kerja program:
### Sender
1. Sender membaca data yang akan dikirim (dari file eksternal) ke sebuah *buffer*.
2. Data dari *buffer* akan di serialize ke dalam array of char `msg_frame` sesuai format Frame (SOH (1), Sequence Number (4), Data Length (4), Isi Data)
3. Dilakukan checksum pada isi `msg_frame` dan menambahkan hasil checksum (1 byte) pada bagian akhir array tersebut.
4. Dilakukan pengiriman frame tersebut ke Receiver dengan method `sendto()`. Dalam saat yang bersamaan, Sender menunggu ACK dari Receiver (menjalankan *thread* AckListener()).

5. Pada **Sliding-Window-Protocol**, terdapat data `window_ack_recv[]` yang berisi apakah window ke-i sudah menerima ACK atau belum. Selain itu terdapat `window_send_data_check[i]` yang berisi apakah sudah mengirim/belum, dan `window_send_data_time[i]` yang berisi waktu saat frame dikirim.
6. Selain itu, dilakukan pengecekan untuk apakah sudah timeout tanpa ACK dengan membandingkan selisih waktu kirim dan waktu saat ini dengan TIMEOUT.
7. Frame akan terus dikirim jika belum pernah dikirim atau sudah timeout tanpa menerima ACK.

### Receiver
1. Receiver akan memanggil fungsi `recvfrom()` untuk menerima frame yang dikirim melalui port tertentu, dan menyimpan data yang diterimanya ke `recvbufer[]`
2. Data diatas akan di *unserialize* untuk mendapatkan sequence number, data length, isi file, dan checksum.
3. Kemudian dilakukan checksum untuk mengecek apakah file yang dikirim sesuai.
4. Jika checksum sesuai, akan dikirim ACK ke-(Next Sequence Number), yaitu current seqnum+1.
5. Jika tidak, berarti terdeteksi packet loss dan akan dikirimkan NAK ke-(Next Sequence Number), yaitu LFR+1.
6. Jika penerimaan file telah selesai dengan lengkap akan dilakukan penulisan kedalam file eksternal.

### Pembagian Tugas

1. Yuly Haruka Berliana Gunawan / 13516031
		- Write File
		- Send & Receive Frame
    - Unserialize Frames to Data
		- Sliding Window Protocol in Receiver
		- Generate and Send ACK
    - Generate and Send NAK

2. Haifa Fadhila Ilma / 13516076
		- Serialize Buffer Data into Frame Format
		- Checksum Data in Sender & Received Buffer
		- Send & Receive Frame
	    - Sliding Window Protocol in Sender

2. Nadija Herdwina Putri Soerojo / 13516130
	    - Read File
		- Sliding Window Protocol in Sender
	    - Checksum ACK in Sender
		- Add multithreading in Sending Frame and Receiving ACK
		- Send-frame Validation (if it's must be sent/not)

# Panduan Pengguna {{APP}}

Panduan ini diterjemahkan dengan mesin dan belum diperiksa oleh penutur bahasa Indonesia. Jika ada bagian yang kurang jelas, panduan berbahasa Inggris adalah rujukannya.

{{APP}} adalah studio menulis untuk teks panjang: novel, skenario film, naskah drama, skrip komik, skripsi, dan buku nonfiksi. Anda menulis dalam potongan-potongan kecil (adegan, bagian, catatan), menyusunnya sesuka hati, lalu pada akhirnya *mengompilasi* semuanya menjadi satu naskah.

Panduan ini membahas versi {{VERSION}}. Tekan **F1** kapan saja untuk membukanya, dan panduan akan terbuka pada bagian yang sesuai dengan apa yang sedang Anda lihat: dokumen, papan gabus, kerangka atau Teks Gabungan, binder, tab inspektor yang sedang memegang papan ketik, atau dialog yang sedang terbuka (dialog yang tidak memiliki bagian sendiri membuka bagian awal). Jendela panduan memiliki **daftar isi** di sisi kiri (klik pada judul untuk melompat ke sana) dan **kolom pencarian** di atas teks: tekan **Ctrl+F** untuk menuju ke sana, ketik sebuah kata, lalu **Return** menampilkan hasil berikutnya dan **Shift+Return** hasil sebelumnya (tombol **Berikutnya** dan **Sebelumnya** melakukan hal yang sama); baris di samping kolom itu menyebutkan hasil ke berapa dari berapa yang sedang ditampilkan, dan setiap hasil ditandai di dalam teks. Hal-hal yang ditujukan bagi orang yang membangun {{APP}} (plugin, paket, folder proyek secara rinci, variabel lingkungan) ada di panduan pengembang, **Bantuan > Pemecahan Masalah dan Pengembangan > Panduan Pengembang: Pembaruan dan Pengaya**.

---

## 1. Langkah pertama

### Mendapatkan program

{{APP}} diunduh dari halaman **Releases** pada halaman proyeknya, https://github.com/Shelin-boop/Quillbinder_Collection/releases. Setiap rilis menyertakan program dalam satu berkas (*{{APP}}-&lt;version&gt;-x86_64.AppImage*), pembaruan program bagi mereka yang sudah memiliki AppImage {{APP}} (bagian 17), dan pengaya (berkas *.qbpkg*, bagian 17). Jadikan AppImage dapat dieksekusi (klik kanan > **Properties** > **Permissions** > *Is executable*, atau `chmod +x` di terminal), lalu klik dua kali; **Bantuan > Tambahkan ke Menu Aplikasi** memasukkannya ke menu desktop Anda. Berkas *SHA256SUMS.txt* pada setiap rilis mencantumkan checksum berkas-berkasnya.

**{{APP}} adalah perangkat lunak {{LABEL}}.** Simpanlah cadangan proyek Anda (bagian 15) dan salinan naskah Anda di luar program.

### Memulai proyek

Ketika {{APP}} dimulai tanpa proyek, **jendela awal** terbuka:

- **Proyek Baru**: pilih kategori dan templat (misalnya Kosong, Novel, Novel dengan Bagian, atau Cerita Pendek), ketik nama, pilih lokasi penyimpanan, lalu klik **Buat**. **Format** menentukan cara proyek disimpan: sebagai proyek {{APP}} (standar) atau sebagai proyek Scrivener 3 (*.scriv*), sehingga Anda juga dapat membukanya di Scrivener (bagian 14, *Bekerja dalam format Scrivener*).
- **Proyek Terbaru**: klik dua kali proyek yang pernah Anda kerjakan.
- **Buka yang Lain…**: cari proyek di disk Anda.
- **Impor Proyek Scrivener…**: buat proyek {{APP}} dari proyek Scrivener (lihat bagian 14). Untuk terus mengerjakan proyek Scrivener itu sendiri, buka berkas *.scrivx*-nya dengan **Buka yang Lain…**.

{{APP}} membuka kembali proyek terakhir saat dimulai (Anda dapat mematikannya di **Berkas > Opsi > Umum**).

**Start pertama** menampilkan satu halaman dengan semuanya sudah terisi (**OK** menyimpannya): pencadangan (apakah proyek dicadangkan saat ditutup, berapa banyak cadangan yang disimpan, 25 pada awalnya, dan di mana), dari AppImage Linux pertanyaan apakah {{APP}} akan ditambahkan ke menu aplikasi (dan apakah AppImage dipindahkan ke *~/Applications*), serta catatan tentang pemeriksaan pembaruan beserta sakelarnya. Proyek selamat datang terbuka saat Anda menekan **OK**; tidak ada jendela program kosong yang tampil sebelumnya. Semuanya dapat diubah nanti di **Berkas > Opsi**; di sana, di bawah **Umum > Startup**, juga terdapat pilihan pengaturan mana yang didahulukan, pengaturan milik proyek atau selalu pengaturan program (bagian 16, *Pengaturan untuk satu proyek*).

### Proyek selamat datang dan Panduan Mulai Cepat

Saat {{APP}} dimulai untuk pertama kalinya, program membuka **proyek selamat datang**: proyek singkat yang menunjukkan, satu dokumen untuk tiap hal, apa saja yang dimiliki {{APP}} di luar yang biasa (bilah ikon, zoom untuk setiap bagian jendela, peta pikiran, lembar objek, komentar satu folder penuh, Bandingkan dan Transfer, proyek Scrivener, tema, pembaruan dan pengaya, serta apa yang melindungi teks Anda). Mulailah dengan dokumen **Welcome** di bagian atas binder; tautannya mengarah ke setiap perhentian dalam tur ini. Tidak ada yang Anda lakukan di proyek selamat datang yang dapat merusak proyek Anda sendiri. (Proyek yang diberikan pada baris perintah, atau berkas proyek yang diklik dua kali, akan dibuka sebagai gantinya; proyek selamat datang kemudian muncul pada start berikutnya.)

Proyek selamat datang disimpan sebagai "Welcome to {{APP}}" di folder *Documents* Anda. **Bantuan > Proyek Selamat Datang…** membukanya kembali seperti yang Anda tinggalkan, atau membuat salinan baru untuk memulai dari awal (**Buat Salinan Baru**; salinan lama masuk ke tempat sampah desktop). Hapuslah kapan pun Anda mau; perintah itu akan membuat yang baru.

**Bantuan > Panduan Mulai Cepat** menunjukkan dalam satu halaman di mana pengaturan-pengaturan penting berada (tema dan mode gelap, warna dan font, pemformatan, zoom, bilah alat, pintasan, cadangan ...) dan merangkum apa saja yang dapat dilakukan {{APP}}.

### Menyimpan

Anda tidak perlu menyimpan secara manual. {{APP}} menyimpan otomatis beberapa detik setelah Anda berhenti mengetik, saat Anda beralih ke program lain, dan saat Anda menutup proyek. Jika Anda terus mengetik, penyimpanan terjadi pada jeda singkat pertama (sekadar menarik napas di antara kata) setelah 5 detik berlalu, dan paling lambat setelah 10 detik walaupun tanpa jeda. **Ctrl+S** menyimpan seketika jika Anda ingin memastikan.

Setiap berkas ditulis lebih dulu dengan nama sementara dan baru menggantikan berkas lama setelah selesai ditulis utuh dan komputer telah menaruhnya di disk. Jika disk penuh atau drive gagal saat menyimpan, berkas lama tetap seperti semula dan perubahan Anda tetap berada di jendela sampai penyimpanan berhasil lagi.

**Status penyimpanan selalu tampak di layar** (0.70). Bilah kaki editor pertama (ruang kosong antara total folder dan jumlah kata, atau di sisi kiri pada bilah kaki lainnya) menampilkan **Tersimpan**, **Belum tersimpan** (beberapa detik setelah Anda mengetik), atau, di atas latar merah, **Tidak tersimpan: penyimpanan terakhir gagal pada 14:03**; selama penyimpanan gagal, judul jendela juga diberi tanda. Penyimpanan yang gagal juga menampilkan pesan yang menyebutkan dokumen yang tidak tersimpan dan menawarkan **Simpan Teks di Tempat Lain…**; pesan itu tidak pernah mengambil alih papan ketik, sehingga tombol berikutnya yang Anda ketik tetap masuk ke teks Anda. Program mencoba lagi sendiri (mula-mula setiap 15 detik, lalu setiap menit), pesan muncul lagi setelah lima menit gagal, dan tanda merah bertahan sampai penyimpanan berhasil. Peta pikiran disimpan dan diperiksa dengan cara yang sama: papan yang tidak dapat disimpan dipertahankan di jendela, dihitung sebagai belum tersimpan saat Anda menutup proyek, dan masuk ke salinan darurat.

**Pesan singkat** (0.93.0). Tidak ada baris bilah status di bagian bawah jendela. Pesan-pesan yang dulu tampil di sana ("Saved again: all changes are on the disk.", hasil pencadangan, dan sejenisnya) muncul selama beberapa detik di tempat yang sama dengan teks status penyimpanan, yaitu di bilah kaki editor pertama (yang kiri atau atas bila ada dua), dan teks status penyimpanan kembali setelahnya. Pesan yang terlalu panjang untuk tempat itu dipersingkat; arahkan penunjuk ke atasnya untuk membacanya utuh. Pada editor yang sangat sempit, teks diganti dengan tanda kecil berwarna (hijau tersimpan, kuning belum tersimpan, merah gagal) dengan teks itu sebagai tooltip-nya. Dalam mode menulis, dan selama tidak ada proyek yang terbuka, tidak ada yang ditampilkan.

**Berkas yang tidak akan ditimpa {{APP}}** (0.70). Berkas teks yang rusak (berkas RTF yang bagian awalnya rusak atau terpotong) atau yang disimpan oleh program lain dengan pengodean karakter lain (berkas Markdown yang disimpan sebagai Windows-1252 atau UTF-16, misalnya setelah diedit di luar program) ditampilkan sejauh dapat dibaca dan tidak dapat diedit; sebuah pesan menyatakannya, dan berkas itu tidak pernah diubah. Salin teksnya ke dokumen baru untuk melanjutkan, atau simpan berkas dalam UTF-8 di program yang menulisnya, lalu tutup dan buka kembali proyek; atau kembalikan ke salah satu snapshot-nya (sejak 0.81.1): berkas yang rusak lebih dulu disimpan sebagai salinan di *settings/conflicts* pada folder proyek, dan teks snapshot menjadi teks dokumen, yang dapat diedit dan disimpan lagi. Gabungkan Dokumen dan Pisahkan menolak dokumen seperti itu dengan menyebutkan alasannya, dan Ganti di Proyek, Konversi Tanda Kutip, serta perubahan gaya membiarkannya apa adanya dan menyebutkannya. Jika sebuah perintah tetap mengubahnya, teks yang berubah tidak hilang: teks itu disimpan sebagai berkas RTF di *settings/conflicts*, dan sebuah pesan menyebutkan tempatnya (0.81.1). Berkas peta pikiran yang tidak dapat dibaca diperlakukan dengan cara yang sama. Karakter yang tidak dapat disimpan (karakter kontrol dari data yang ditempel) dibuang seperti sebelumnya; pesan singkat di bilah kaki kini menyatakannya satu kali per dokumen.

**Penyimpanan selalu mendarat secara utuh** (0.65). Ketika satu penyimpanan menulis beberapa berkas (teks dokumen baru dan berkas proyek, teks beserta gambarnya, lembar Objek), semuanya lebih dulu ditulis di samping tempatnya, lalu dicatat dalam jurnal kecil (*settings/journal.json*), dan baru kemudian ditaruh di tempatnya bersama-sama. Jika program atau komputer berhenti di tengah jalan, pembukaan berikutnya menyelesaikan penyimpanan itu dari jurnal, atau, jika jurnal belum lengkap, membiarkan proyek seperti pada penyimpanan sebelumnya; sebuah pemberitahuan menyebutkan yang mana. Jurnal yang tidak dapat dibaca (kerusakan disk) tidak pernah dianggap kosong: jurnal itu dan berkas-berkas penyimpanannya disimpan dalam sebuah folder di *settings/conflicts*, tidak ada yang ditaruh di tempatnya atau dihapus, dan pemberitahuan menyatakannya (0.81.1). Di folder sinkronisasi, jurnal dari penyimpanan komputer lain dibiarkan selama 15 menit (penyimpanan itu mungkin masih berjalan di sana, atau berkas-berkasnya masih berdatangan): selama itu komputer ini tidak menyimpan proyek dan mencoba lagi sendiri, dan jika komputer lain berhenti saat penyimpanan itu, komputer ini menyelesaikannya setelah 15 menit tersebut (0.81.1). Jika satu berkas dari sebuah penyimpanan tidak dapat ditulis, tidak satu pun menggantikan apa pun, dan perubahan Anda tetap berada di jendela untuk percobaan berikutnya. Sejak 0.93.0, drive yang melaporkan kesalahan ketika sebuah folder disinkronkan setelah berkas-berkas ditaruh di tempatnya (kesalahan sungguhan, bukan sistem berkas yang tidak dapat menyinkronkan) membuat penyimpanan dihitung gagal: jurnal tetap ada, pesan menyatakan bahwa drive melaporkan kesalahan, dan penyimpanan atau pembukaan berikutnya menyelesaikan penyimpanan itu dari jurnal. Selama jurnal yang tidak dapat dibaca belum dapat disisihkan (folder *settings/conflicts* tidak dapat ditulis), tidak ada penyimpanan yang menulis jurnal baru di atasnya: penyimpanan menunggu, pesannya menyebutkan berkas itu, dan program menyimpan sendiri begitu berkas itu dapat disisihkan. Berkas sementara dari penyimpanan sebelumnya yang menghalangi dipindahkan ke samping, tidak pernah ditimpa. Sinkronisasi folder yang gagal sebelum ada yang diganti adalah kegagalan yang bersih: tidak ada yang berubah. Jika kesalahan muncul setelah berkas-berkas berada di tempatnya, program mencatat apa yang ada di disk (agar percobaan ulang tidak melaporkan konflik "diubah di tempat lain" yang keliru dan tidak menulis gambar dua kali), dan menyinkronkan folder lagi pada setiap penyimpanan: "Tersimpan" selalu mengikuti sinkronisasi folder yang telah diperiksa. Sistem berkas yang menjawab sinkronisasi folder dengan kesalahan sejak proyek dibuka diingat sebagai sistem yang tidak dapat menyinkronkan folder, dan penyimpanannya tidak dinyatakan gagal.

Jika Anda keluar dari sesi atau mematikan komputer saat {{APP}} terbuka, program menyimpan semuanya sebelum berakhir (tidak ada cadangan yang dibuat saat itu). Jika penyimpanan itu gagal (disk penuh, drive yang hilang), teks dan catatan yang belum tersimpan (dan, sejak 0.93.0, berkas proyek seperti yang ada di memori) ditulis sebagai berkas RTF ke dalam folder darurat, *~/.local/share/{{APP}}/Emergency*, dan start berikutnya menyebutkan lokasinya, serta berapa banyak berkas yang ditulis (pemberitahuan menyebut "tidak lengkap" dan menyebutkan apa yang kurang bila ada berkas yang tidak dapat ditulis). Jika program berakhir tanpa diduga, start berikutnya memberi tahu Anda (bagian 20, *Laporan crash*).

**Menutup proyek yang tidak dapat disimpan** menanyakan apa yang harus dilakukan; **Tetap Buka** adalah pilihan bawaan. **Simpan Teks di Tempat Lain…** menulis teks dan catatan yang belum tersimpan sebagai berkas RTF ke folder yang Anda pilih (drive lain, flash disk USB), dan proyek tetap terbuka. Sejak 0.93.0 salinan darurat ini juga memuat berkas proyek seperti yang ada di jendela (binder, judul, dan label), papan peta pikiran, dan objek yang berubah, dan salinan dibuat bahkan ketika hanya binder atau Objek yang berubah. Pesan menyebutkan berapa banyak berkas yang ditulis, dalam bentuk "n dari m", serta menyebutkan berkas yang hilang beserta alasannya; salinan yang kekurangan berkas diberi judul **tidak lengkap** (dengan tanda peringatan) dan tidak pernah tampak seperti salinan yang selesai. Folder yang sudah ada tidak pernah dipakai lagi: salinan kedua dalam detik yang sama diberi nomor. Berkas proyek dalam salinan itu diberi nama seperti berkas proyek dengan tambahan ".copy" (agar folder tidak keliru dianggap proyek); README dalam salinan menjelaskan cara mengembalikannya: tutup proyek, ganti nama berkas proyek di folder proyek (berkas itu tetap seperti semula), salin berkas dari salinan darurat ke sana, ganti namanya menjadi nama berkas proyek, lalu buka proyek lagi. Untuk proyek dalam format Scrivener, README menyatakan bahwa berkas itu milik salinan kerja dan tidak memberikan jalan kembali. Jika ternyata tidak ada yang belum tersimpan (percobaan ulang berhasil sementara itu), pesan menyatakan semuanya sudah tersimpan dan tidak ada salinan yang diperlukan. **Tutup Tanpa Menyimpan** menutupnya dan kehilangan perubahan tersebut.

### Di mana pekerjaan Anda berada

Proyek adalah folder bernama *JudulAnda*{{SUFFIX}}. Di dalamnya, setiap dokumen adalah berkas teks biasa, sehingga tulisan Anda tetap dapat dibaca meski tanpa {{APP}}. Klik dua kali berkas *JudulAnda*{{FILE}} di dalam folder untuk membuka proyek. Jangan mengganti nama atau memindahkan berkas di dalam folder proyek secara manual.

Untuk menemukan berkas sebuah dokumen di luar {{APP}}, buka *contents-by-title.txt* di folder proyek (0.65): berkas itu mencantumkan setiap dokumen binder dengan judulnya, menjorok sesuai tempatnya, dan berkas-berkas yang menyimpan teks, catatan, atau medianya (misalnya *content/<id>/text.md*). {{APP}} menulisnya pada setiap penyimpanan yang mengubah binder dan tidak pernah membacanya sendiri. (Proyek dalam format Scrivener tidak memiliki daftar semacam itu; *.scrivx* milik Scrivener menyebutkan dokumen-dokumennya.)

Sebuah proyek sebaiknya terbuka hanya di satu jendela pada satu waktu. Jika proyek itu sudah terbuka di jendela lain atau di komputer lain (misalnya melalui folder yang disinkronkan), {{APP}} memperingatkan Anda sebelum membukanya. Memulai {{APP}} lagi dengan proyek yang sedang terbuka di jendela program (dari pengelola berkas atau pintasan desktop) membawa jendela itu ke depan alih-alih membuka proyek dua kali; proyek lain terbuka di jendelanya sendiri.

**Proyek di folder yang disinkronkan** (OneDrive, Dropbox, Nextcloud, Syncthing, atau program sinkronisasi lain apa pun) dapat dipakai di beberapa komputer, satu per satu. Tutup di satu komputer dan biarkan program sinkronisasi selesai sebelum Anda membukanya di komputer lain. {{APP}} membantu hal itu pada tingkat berkas, apa pun program sinkronisasinya:

- Selama proyek terbuka, berkas kunci kecil di dalamnya menyebutkan nama komputer dan diperbarui setiap beberapa menit. Berkas itu memuat nama komputer, nama pengguna Anda di komputer itu, versi dan nomor proses program, kapan proyek dibuka dan kapan kunci terakhir diperbarui, serta kode yang membedakan komputer satu dari yang lain (hash, tidak terbaca sebagai nama). Semua orang yang memiliki akses ke folder bersama dapat melihatnya. Di **Berkas > Opsi > Umum > Menyimpan**, **Nama komputer ini di folder bersama** menetapkan nama Anda sendiri ("Laptop"); nama itu dipakai sebagai ganti nama komputer di berkas kunci, di nama versi yang disimpan, dan di *settings/ui-<computer>.json*, dan nama pengguna Anda kemudian tidak disertakan. Di komputer lain, membuka proyek kemudian menampilkan nama komputer itu, sejak kapan proyek terbuka di sana, dan tanda kehidupan terakhirnya; **Tetap Buka** atau **Batal**. Kunci yang tidak diperbarui selama lebih dari 15 menit (proyek tidak ditutup di sana, atau komputer itu mati) ditampilkan sebagai sisa, dan **Buka** adalah pilihan bawaan.
- Jika proyek dibuka di komputer lain sementara masih terbuka di sini, sebuah pemberitahuan menyatakannya dan menawarkan untuk menutupnya di sini.
- Jika sebuah berkas proyek diubah di tempat lain (di komputer lain melalui program sinkronisasi, atau oleh program lain) setelah {{APP}} membacanya, dan berkas itu juga diubah di sini, versi yang lain disimpan sebelum versi Anda disimpan: sebagai **snapshot** dokumen (bandingkan di tab Snapshot pada inspektor) dan sebagai salinan di folder proyek *settings/conflicts*. Sebuah pemberitahuan menyebutkan dokumen mana. Jika versi yang lain tidak dapat disimpan (misalnya karena disk penuh atau *settings/conflicts* tidak dapat ditulis), versi Anda tidak disimpan menimpanya: penyimpanan gagal dengan pesan yang menyebutkan berkasnya, dan perubahan Anda tetap belum tersimpan sampai versi yang lain dapat disimpan. Dokumen yang hanya Anda lihat di sini tidak pernah disimpan menimpa versi yang lebih baru dari tempat lain.
- Membuka dan menutup proyek tanpa mengubah apa pun tidak mengubah satu pun berkasnya; tata letak jendela dan tempat Anda berada di setiap dokumen disimpan per komputer (*settings/ui-<computer>.json*). Dalam format Scrivener, *.scriv* juga tidak disentuh, dan mendapat berkas kunci milik {{APP}} (*quillbinder.lock*) selama terbuka; Scrivener tidak membacanya. Sebelum setiap penyimpanan ke dalam *.scriv*, {{APP}} memeriksa kunci milik Scrivener sendiri: jika Scrivener membuka proyek setelah {{APP}}, tidak ada lagi yang ditulis ke dalam *.scriv* (sebuah pemberitahuan menyatakannya satu kali, lalu pesan singkat di bilah kaki); perubahan Anda tetap berada di salinan kerja {{APP}}. Tutup proyek di Scrivener dan simpan lagi (**Berkas > Simpan**). Jika Anda menutup proyek di sini lebih dulu, perubahan masuk ke *.scriv* saat Anda membukanya di sini berikutnya; bila Scrivener mengubah sesuatu selama itu, versinya yang dibaca.
- Jika proyek mencantumkan teks untuk sebuah dokumen yang berkasnya belum tiba di komputer ini (program sinkronisasi belum selesai), sebuah pemberitahuan menyebutkannya setelah proyek dibuka; dokumen itu ditampilkan kosong, tidak dapat diedit, dan tidak ada yang disimpan sebagai gantinya. Tutup proyek, tunggu sampai program sinkronisasi selesai, lalu buka lagi. Hal yang sama berlaku untuk teks yang hanya dapat dibaca sebagian (0.65).
- Berkas yang diubah di tempat lain juga terdeteksi ketika ukuran dan waktunya tetap sama (beberapa program sinkronisasi menetapkan waktunya sendiri): {{APP}} membandingkan isinya dengan apa yang dibaca atau ditulisnya (0.65).
- Isi juga menentukan ke arah sebaliknya (0.92.0): berkas yang **hanya waktunya** berubah bukanlah versi lain. Beberapa program sinkronisasi menetapkan ulang waktu berkas beberapa detik setelah {{APP}} menulisnya, ketika pengunggahan mereka selesai; tidak ada yang disimpan untuk itu, tidak ada pemberitahuan, dan berkas tidak ditulis lagi. Berkas proyek yang sudah memuat apa yang akan ditulis oleh sebuah penyimpanan juga dibiarkan, sehingga penyimpanan yang tidak mengubah apa pun tidak memberi program sinkronisasi apa pun untuk dibawa.
- Pemberitahuan **Versi Lain Disimpan** memiliki tombol **Jangan Tampilkan Lagi Sampai {{APP}} Ditutup**. Versi-versi lain tetap disimpan sebagai snapshot dan sebagai salinan di *settings/conflicts*; hanya pemberitahuannya yang tidak muncul sampai Anda keluar dari program. Start berikutnya menampilkannya lagi.
- **Gembok pada apa yang sedang dikerjakan perangkat lain (0.92.0).** Selama proyek terbuka, setiap perangkat juga mencatat, di samping berkas kunci, dokumen mana yang dibukanya di editor atau yang diubahnya sejak proyek dibuka. Ketika perangkat kedua membuka proyek, binder-nya menampilkan gembok kecil di ujung kanan baris dokumen-dokumen itu, dan pada folder di sekitarnya. Arahkan penunjuk ke gembok: tooltip menyebutkan perangkat itu dan kapan perangkat itu membuka proyek. Ini hanyalah petunjuk, bukan penjaga: tidak ada yang diblokir, dan Anda tetap dapat mengedit dokumen itu. Tandanya berjalan melalui program sinkronisasi Anda, sehingga bisa berusia beberapa menit; tanda ini memerlukan 0.92.0 atau lebih baru di kedua perangkat (Scrivener tidak mengumumkan apa pun). Daftar dari perangkat yang tidak menunjukkan tanda kehidupan selama lebih dari 15 menit tidak ditampilkan. Catatan itu adalah berkas kecil per perangkat (*settings/lock-work-…json*; untuk proyek dalam format Scrivener *quillbinder.lock-work-…json* di dalam *.scriv*); isinya nama perangkat seperti di berkas kunci, waktu, dan nomor internal dokumen, tanpa judul dan tanpa teks. Berkas ini hanya ditulis ketika daftar berubah, tidak pernah per ketikan, dihapus saat proyek ditutup, dan tidak disertakan dalam cadangan.
- **Format Scrivener, dua komputer** (0.65): dokumen yang ditambahkan ke *.scriv* di tempat lain (Scrivener di komputer lain) selagi proyek terbuka di sini dipertahankan, dan binder yang menyebutkannya tidak digantikan; sebuah pemberitahuan meminta Anda menutup proyek dan membukanya lagi, yang kemudian menampilkan dokumen mereka dan dokumen Anda. Gambar, PDF, berkas audio dan video, serta peta pikiran yang diubah di *.scriv* di tempat lain dipertahankan apa adanya (sebelumnya, versi di sini disalin menimpanya); bila salah satunya diubah di kedua sisi, versi yang lain disimpan lebih dulu di *settings/conflicts*.

Proyek dalam **format Scrivener** adalah folder *JudulAnda.scriv* sebagai gantinya; bukalah dengan berkas *.scrivx*-nya. {{APP}} menyimpan perubahan Anda ke dalamnya, sehingga proyek tetap menjadi proyek Scrivener. **Berkas > Simpan Sebagai…** menyimpan proyek dengan nama lain atau di tempat lain, dalam kedua format (bagian 14).

### Privasi: apa yang keluar dari komputer Anda

{{APP}} menghubungi internet hanya untuk satu hal: memeriksa pembaruan. Program mengambil satu berkas kecil, tidak mengirim apa pun tentang Anda, dan Anda dapat mematikannya di menu Bantuan. (Bagian 17, *Memeriksa pembaruan*, menjelaskan dengan tepat apa yang diambil dan kapan. Sebelum 0.92.0 program tidak memeriksa pembaruan sama sekali.) Tiga hal lain melalui internet, dan hanya ketika Anda memulainya:

- **Berkas > Impor > Halaman Web…** mengambil halaman yang Anda sebutkan beserta gambar-gambarnya dari situs itu. Seperti peramban, program mengirim nama peramban umum (Firefox di Linux), bahasa sistem Anda (untuk bahasa halaman) dan, seperti setiap koneksi, alamat internet komputer Anda.
- **Mengunduh model bahasa** untuk pengaya Asisten Penerjemah (bagian 10) mengambilnya dari rilis repositori {{APP}} di GitHub. Program mengirim nama pengaya ("Quillbinder-Translation-Addon") dan, seperti setiap koneksi, alamat internet Anda.
- **Mengunduh model ucapan** (pengaya Transkripsi Ucapan, bagian 10) mengambil model dari Hugging Face (huggingface.co, perusahaan di Amerika Serikat) atau dari alamat yang Anda tetapkan di opsinya. Program mengirim "{{APP}}/<version>" sebagai nama program dan, seperti setiap koneksi, alamat internet Anda. Sebuah model hanya dipasang bila checksum-nya adalah checksum yang dikenal; berkas dari alamat lain atau yang Anda unduh sendiri hanya dipakai setelah Anda menyetujuinya.

Apa yang dapat dilihat orang lain dari pekerjaan Anda:

- **Proyek di folder bersama** membawa berkas kunci selama terbuka (lihat di atas): nama komputer, nama pengguna, dan waktu. Tetapkan nama Anda sendiri di **Berkas > Opsi > Umum > Menyimpan** untuk menghilangkan hal-hal itu.
- **Informasi Penulis** (nama, alamat, telepon, e-mail di **Berkas > Opsi**) tetap berada di pengaturan program di komputer ini; informasi itu tidak pernah masuk ke pengaturan milik proyek. Informasi itu sampai ke naskah hanya bila format kompilasi memakai placeholder (<$author>, <$address> ...).
- **Teks yang dihapus belum tentu hilang di mana-mana**: snapshot menyimpan versi-versi sebelumnya dari sebuah dokumen (**Proyek > Hapus Snapshot Lama** menghapusnya), dan cadangan menyimpan seluruh proyek seperti adanya saat itu (bagian 15). Cadangan adalah berkas ZIP biasa tanpa kata sandi; folder cadangan di dalam folder yang disinkronkan akan diunggah oleh program sinkronisasi.
- **Berkas Anda tidak dienkripsi sampai Anda memintanya** (0.99.0: **Berkas > Opsi > Enkripsi**, lihat bagian 15; sebelumnya tidak ada yang dienkripsi): teks, catatan, pengaturan proyek, dan cadangan adalah berkas biasa, dapat dibaca oleh siapa pun yang dapat membaca folder tempatnya berada. Untuk menjaga naskah tetap pribadi, simpanlah proyek dan cadangannya di disk terenkripsi (penginstal Kubuntu menawarkan enkripsi seluruh disk; untuk flash disk USB atau drive eksternal, Plasma Vaults milik KDE atau drive terenkripsi LUKS), dan jauhkan folder cadangan dari folder bersama yang tidak Anda kendalikan.
- **Laporan crash** tetap berada di komputer ini sampai Anda menyalin atau mengirimnya sendiri (bagian 20). Laporan itu menggambarkan keadaan program dan menyebutkan proyek yang terbuka beserta foldernya. Di Windows, dump memori disimpan di samping setiap laporan; dump itu dapat memuat teks yang sedang Anda tulis, dan laporan menyatakannya.

### Menambahkan {{APP}} ke menu aplikasi

Saat Anda pertama kali memulai berkas program ({{APP}}.AppImage), halaman start pertama menawarkan untuk menambahkan {{APP}} ke menu aplikasi. Setelah itu Anda dapat memulainya seperti program lain, dan klik dua kali pada proyek membukanya. Tidak ada kotak yang mengonfirmasi entri itu; kegagalan akan diberitahukan. Halaman yang sama (dan **Bantuan > Tambahkan ke Menu Aplikasi**) menawarkan untuk **memindahkan AppImage ke ~/Applications** (awalnya dicentang bila berkasnya belum berada di sana), agar entri menu menjalankannya dari tempat tetap tempat berkas itu berada (kotak centang menyebutkannya dengan kata-kata ini, tanpa tanda kurung); jika pemindahan gagal (disk penuh, sudah ada berkas dengan nama itu), tidak ada yang hilang: berkas tetap di tempatnya, entri mengarah ke sana, dan pesan menyebutkan alasannya. Jika nanti Anda memindahkan AppImage sendiri, entri itu masih menyebut tempat lama: pakai **Bantuan > Tambahkan ke Menu Aplikasi** sekali lagi, yang menuliskan tempat berkas saat ini (entri lama diganti, tidak digandakan). Setelah dipindahkan, pembaruan program, salinan *.before-update*, dan **Bantuan > Pemecahan Masalah dan Pengembangan > Informasi Sistem** langsung memakai tempat yang baru.

Satu perintah menu Bantuan, **Bantuan > Tambahkan ke Menu Aplikasi**, mengikuti keadaan entri: perintah itu menambahkan entri selama belum ada dan, setelah ada, bernama *Hapus dari Menu Aplikasi* dan menghapus apa yang dibuat oleh penambahan (berkas entri, ikon, dan jenis berkas) serta membiarkan AppImage di tempatnya. Jika Anda memindahkan berkas program sendiri, hapus entrinya dan tambahkan lagi.

### Di Windows: semuanya dalam satu folder

Versi Windows {{APP}} (eksperimental, dipublikasikan sejak 0.91.1) adalah folder yang Anda ekstrak di mana pun Anda suka, misalnya *C:\Users\<your name>\Quillbinder*; tidak perlu instalasi dan tidak perlu hak administrator. Pengaturan, pengaya, pembaruan program, font terpasang, dan laporan crash disimpan di folder *data* di dalam folder itu, dan tidak ada apa pun di folder bawaan Windows atau registri. Seluruh folder dapat dipindahkan atau disalin (ke flash disk USB atau komputer lain); menghapusnya berarti menghapus program. Folder yang tidak dapat ditulis oleh program (seperti *Program Files*) ditolak dengan pesan yang menyebutkan di mana sebaiknya folder itu ditaruh. Pembaruan program dipasang dengan **Bantuan > Pengaya dan Pembaruan > Instal Pembaruan atau Pengaya**; untuk berpindah ke versi yang baru Anda unduh, ekstrak dan salin folder *data* ke folder yang baru. Proyek Anda tetap berada di mana pun Anda menyimpannya.

Ada dua cara untuk mendapatkan folder itu (0.90.4). **Program setup** (*...-setup.exe*) menanyakan folder dan mengekstrak {{APP}} ke dalamnya; program ini tidak memerlukan hak administrator dan tidak mengubah apa pun di komputer: tidak ada entri di bawah Apps, tidak ada apa pun di registri, tidak ada entri menu Start (panduan yang dijelaskan di bawah menawarkannya). Mengekstrak versi yang lebih baru ke folder yang sama mempertahankan *data*, yaitu pengaturan dan pengaya Anda. **Berkas zip** memuat folder yang sama: ekstrak sepenuhnya. Windows mungkin memperingatkan tentang kedua berkas karena tidak ditandatangani (SmartScreen: *More info*, lalu *Run anyway*).

Dua hal berbeda dari Linux karena Windows itu sendiri (0.90.4). Tombol **AltGr** mengetik karakternya, seperti yang Anda harapkan: pada papan ketik Jerman AltGr+7, 8, 9, 0 dan < menghasilkan { [ ] } dan |, meskipun {{APP}} memiliki perintah pada Ctrl+Alt dengan tombol-tombol ini; tekan tombol Ctrl dan Alt kiri untuk perintah-perintah tersebut. Dan berkas yang hanya diperlukan sesaat (halaman web tersimpan yang diekstrak untuk dilihat, atau salinannya untuk peramban) disimpan di folder sementara Windows dan dihapus ketika {{APP}} ditutup. Transkripsi Ucapan memakai prosesor di Windows, bukan kartu grafis.

Pada start pertama, sebuah panduan menyebutkan di mana program menyimpan berkasnya dan memperingatkan bila foldernya berada di dalam folder layanan sinkronisasi (OneDrive, Dropbox, Google Drive ...) atau memiliki jalur yang panjang. Panduan itu juga menanyakan hal-hal berikut, yang ditampilkan lagi oleh **Bantuan > Pemecahan Masalah dan Pengembangan > Komputer Ini** dan **Berkas > Opsi > Umum > Komputer Ini**:

- **Cadangan**: tidak ada folder cadangan standar. Sampai Anda memilihnya, tidak ada cadangan otomatis yang dibuat; bilah kaki editor pertama menampilkan **Tanpa cadangan** di samping teks status penyimpanan (klik untuk memilih folder), dan pencadangan pertama yang akan berjalan menanyakan folder. Yang terbaik adalah drive lain.
- **Catatan Catatan Sementara dan transkripsi**: di Documents (standar), di folder program, atau di folder pilihan Anda. Perubahan memberikan tempat baru kepada kedua alat; apa yang sebelumnya mereka tulis tetap di tempatnya.
- **Windows**: entri di menu Start, pintasan di desktop, dan jenis berkas proyek (berkas *.qbx* terbuka dengan {{APP}}, dan program ini ditawarkan di bawah **Open with** untuk berkas *.scrivx* milik Scrivener). Hanya inilah yang ditaruh program di luar foldernya, masing-masing hanya bila Anda menambahkannya, dan masing-masing dengan **Hapus**. Hapuslah sebelum Anda menghapus atau memindahkan folder.

Font yang dipasang dengan **Format > Font > Instal Berkas Font** tetap berada di folder program dan hanya milik {{APP}}; program lain tidak melihatnya.

Pengaya dengan kode program memiliki versi Windows sendiri (nama berkasnya berakhiran *-windows*): **Pemutaran Media** memutar apa yang diputar Windows sendiri, **Format Video dan Audio Tambahan** menambahkan FFmpeg, dan **Transkripsi Ucapan** menuliskan ucapan menggunakan prosesor (kartu grafis belum dipakai di Windows). **Asisten Penerjemah** juga menjadi bagian versi lengkap di Windows (0.92.0). Pengaya tanpa kode program (tema, templat, format kompilasi) adalah berkas yang sama di setiap sistem.

### Di tablet Android

Versi Android {{APP}} (eksperimental, dipublikasikan sejak 0.91.3) adalah aplikasi untuk tablet dengan Android 9 atau lebih baru (ARM 64-bit). Aplikasi ini memiliki jendela yang sama seperti di komputer, dibuat untuk jari: satu jari menggulir dan mengetuk, dua jari melakukan zoom, jari yang berhenti setengah detik pada sesuatu membuka menu klik kanannya (di dalam teks, ketukan pada teks yang dipilih melakukannya), dan bilah alat terlipat menjadi sebanyak baris yang dibutuhkan layar. Unduh berkas APK di tablet dan bukalah; Android menanyakan apakah peramban atau aplikasi Files boleh memasang aplikasi, yang Anda izinkan untuk pemasangan ini. Versi baru datang sebagai APK baru dan dipasang di atas yang lama, dengan mempertahankan pengaturan dan pengaya Anda; **Bantuan > Pengaya dan Pembaruan > Periksa Pembaruan Sekarang** memberi tahu Anda bila ada versi baru dan membuka unduhannya di peramban (pengaya yang lebih baru dipasang di dalam {{APP}}).

Dengan papan ketik dan mouse eksternal (sejak 0.92.0): Ctrl + roda mouse melakukan zoom, dan Ctrl + klik atau Shift + klik memilih beberapa dokumen, seperti di komputer. Garis di antara dua panel menampilkan pegangan; mouse yang ditekan beberapa piksel di sampingnya akan membawanya serta. Tombol **Back** Android menutup menu, tip, atau jendela di depan; setiap jendela memiliki tombol yang menutupnya, dan jendela yang lebih besar dari layar dapat digulir. Di tablet, pin peta pikiran yang dikunci tampil pada baris binder yang dipilih (klik, atau gunakan menu klik kanan barisnya: *Kunci Peta Pikiran Item Ini*, *Lepaskan Peta Pikiran yang Dikunci*), dan ketukan pada gembok sebuah baris menunjukkan siapa yang mengerjakannya. Jari pada gambaran umum peta pikiran bekerja seperti mouse: ketukan memindahkan tampilan ke sana, jari menyeret persegi panjang bagian yang ditampilkan, dan pegangan di sudutnya mengubah ukurannya. **Berkas > Simpan Sebagai…** menanyakan format lebih dulu di tablet, lalu membuka jendela berkas.

Pada start pertama {{APP}} meminta akses ke berkas tablet. Dengan akses itu, proyek masuk ke *Documents/Quillbinder*, cadangan ke *Documents/Quillbinder/Backups*, ekspor ke *Documents/Quillbinder/Exports* dan catatan Catatan Sementara ke *Documents/Quillbinder/Scratch Pad*, tempat aplikasi lain, aplikasi sinkronisasi, dan komputer yang tersambung dengan kabel dapat menjangkaunya; tombol membuka halaman Android *All files access*, tempat Anda mengaktifkannya untuk {{APP}}. Tanpa akses itu, semuanya tetap berada di folder milik aplikasi, yang tidak dilihat aplikasi lain dan yang dihapus Android bersama aplikasinya; *File > Files on This Tablet* menanyakan lagi dan menyebutkan di mana semuanya berada. Ketika Android menyingkirkan aplikasi (tombol Home, aplikasi lain), {{APP}} langsung menyimpan.

Kamus ejaan (Inggris dan Jerman; bahasa lainnya berupa pengaya, lihat bagian 4), alat menulis, pengimpor halaman web dan berkas Word lama, dan (sejak 0.92.0) Asisten Penerjemah adalah bagian dari aplikasi; Asisten Penerjemah dipasang pada start pertama sebuah versi dan hanya mengunduh model bahasa bila Anda memilihnya. Pengaya untuk Android (nama berkasnya berakhiran *-android-arm64*) dipasang dengan **Bantuan > Pengaya dan Pembaruan > Instal Pembaruan atau Pengaya**; jendela berkas terbuka di folder Download, dan {{APP}} dimulai lagi bila pengaya memerlukannya. Paket yang dibuat untuk Linux atau Windows dan pembaruan program ditolak dengan alasannya: aplikasi itu sendiri hanya diperbarui oleh APK baru. Belum ada di versi Android: Transkripsi Ucapan, audio dan video di editor, dan laporan crash. Teks yang disalin di dalam {{APP}} mempertahankan pemformatannya saat ditempel; papan klip Android hanya membawa teks biasa, sehingga teks yang disalin ke aplikasi lain tiba sebagai teks biasa.

---

## 2. Jendela utama

Jendela memiliki tiga kolom:

| Area | Fungsinya |
|------|-----------|
| **Binder** (kiri) | Daftar isi seluruh proyek Anda: setiap dokumen dan folder. |
| **Editor** (tengah) | Menampilkan apa yang Anda pilih di binder: teks, papan gabus berisi kartu indeks, atau kerangka. |
| **Inspektor** (kanan) | Informasi tentang dokumen saat ini: sinopsis, catatan, penanda, metadata, snapshot, dan komentar. |

Saat {{APP}} pertama kali dimulai pada layar yang lebih sempit dari 1600 px, jendela terbuka dalam keadaan dimaksimalkan. Ukuran, tempat, dan keadaan jendela (dimaksimalkan atau tidak) disimpan sesaat setelah setiap perubahan, sehingga crash tidak menghilangkannya. Sembunyikan binder dengan **Ctrl+Alt+B** dan inspektor dengan **Ctrl+Alt+I** untuk mendapatkan ruang lebih luas. **Bagian yang memegang papan ketik.** Garis jelas setebal 2 px dalam warna sorotan tema mengelilingi bagian jendela yang memegang papan ketik (binder, editor, papan gabus, kerangka, inspektor, atau pemegang salinan di dalam editor), di setiap tema; hanya ada satu garis pada satu waktu. Itu adalah jendela yang selebar tingginya; jendela yang lebih tinggi daripada lebarnya menata ketiga area yang sama secara berbeda (bagian berikutnya). Warna dan font ketiganya, serta papan gabus, kerangka, dan mode menulis, berasal dari tema (**Tampilan > Tema**) dan dapat diubah area demi area (bagian 16), juga dengan klik kanan pada area itu (**Penampilan**). Setiap proyek menyimpan tampilannya sendiri.

**Bilah alat** di bagian atas memuat, dari kiri ke kanan: tombol **Tampilan** (binder, koleksi, inspektor, tata letak), dokumen baru (dengan menu untuk folder, templat, dan impor), folder baru, tempat sampah, **urungkan** dan **ulangi**, tombol **cari** (pencarian proyek; panah di sampingnya mencantumkan alat pencarian lain: Cari, Ganti di Proyek, Cari menurut Pemformatan), kolom **pencarian cepat**, ketiga tombol tampilan (teks, papan gabus, kerangka), penanda proyek, referensi cepat, komentar dan catatan kaki, kompilasi, mode menulis, **mikrofon** Transkripsi Ucapan (bila pengaya itu terpasang), dan tombol inspektor. Anda menentukan apa yang dimuatnya (lihat "Bilah alat buatan Anda sendiri" di bawah).

**Bilah format** di bawahnya memuat gaya, font, ukuran, jarak baris, pemformatan teks bawaan, jendela font, tebal, miring, garis bawah, coret, sorot, warna teks, perataan, daftar, spasi sebelum dan sesudah paragraf, indentasi baris pertama dan seluruh paragraf, komentar, catatan kaki, anotasi sebaris, dan gambar. Tampilkan atau sembunyikan kedua bilah itu dengan **Tampilan > Bilah Alat**.

**Dialog.** Jendela yang dibuka program (Opsi, Kompilasi, Target Proyek ...) tidak pernah lebih besar daripada layar tanpa panel desktop: pada layar laptop yang kecil, isinya menggulir atau mengecil, dan tombolnya tetap terlihat. Ketika ditutup, papan ketik kembali ke tempat semula, biasanya teks, sehingga Anda dapat langsung mengetik lagi.

**Di mana saya berada?** Klik dua kali ruang di samping teks (kertas di samping kolom atau area di sekitarnya): binder memilih dokumen di samping klik itu, membuka folder-folder di atasnya, dan membuat barisnya berkedip dua kali dengan warna oranye. Editor mempertahankan apa yang ditampilkannya. **Navigasi > Tunjukkan di Binder** (**Ctrl+Shift+R**) melakukan hal yang sama untuk dokumen di kursor.

**Kaki** setiap editor menampilkan jumlah kata dan karakter (pilih teks untuk menghitung hanya pilihan itu) dan target untuk dokumen. Klik jumlah itu untuk rinciannya. Ketika editor menampilkan folder (atau dokumen dengan subdokumen), kaki juga menampilkan **total** folder: kata dari teksnya sendiri dan dari semua yang ada di dalamnya, misalnya "Folder: 12,345 words". Papan gabus dan kerangka menampilkan total yang sama di tengah kaki mereka; bila beberapa item ditampilkan, itu adalah total semuanya. Klik pada total menampilkan rinciannya (kata, karakter, karakter tanpa spasi, paragraf, dan halaman), seperti yang dilakukan jumlah sebuah teks. Inspektor menampilkannya di bawah **Total** pada metadata folder, dan mengarahkan mouse ke sebuah folder di binder juga menampilkannya (beserta kata milik folder itu sendiri bila memiliki teks sendiri).

### Menu

| Menu | Isinya |
|------|--------|
| **Berkas** | Proyek, penyimpanan, format proyek ({{APP}} atau Scrivener), cadangan, impor dan ekspor, bandingkan dan transfer dengan berkas Word atau LibreOffice, kompilasi, pencetakan, konversi berkas, opsi |
| **Edit** | Urungkan, papan klip, pemilihan, mencari dan mengganti (di teks, di proyek, menurut pemformatan), memindahkan dokumen di binder, alat menulis |
| **Sisipkan** | Gambar, tabel, tautan ke dokumen dan objek, tautan web, komentar, catatan kaki, stempel waktu media, nomor otomatis, placeholder, karakter khusus, tanggal dan waktu |
| **Tampilan** | Ketiga mode tampilan, editor terpisah (**Pisahkan Editor**), tempat binder, editor, dan inspektor serta tata letak tersimpan (**Tata Letak**), tema warna (**Tema**), binder, koleksi, dan inspektor (serta **Tambah Cepat Tombol di Binder**), bilah alat, opsi pengeditan teks (tampilan halaman, penggaris, pengguliran mesin ketik), opsi papan gabus dan kerangka, warna label, zoom, mode menulis, layar penuh |
| **Navigasi** | Kembali dan maju, dokumen sebelumnya dan berikutnya, membuka dokumen di editor lain, di pemegang salinan, atau sebagai referensi cepat, kunci editor, editor mana yang dimuat binder, tab inspektor (semuanya, dengan nama seperti pada tooltip-nya: **Sinopsis dan catatan**, **Penanda**, **Metadata**, **Snapshot**, **Komentar dan catatan kaki**, peta pikiran, objek, dan tab alat menulis) |
| **Proyek** | Dokumen baru, pengaturan proyek (juga disimpan ke dan dimuat dari berkas), target, kata kunci, statistik, riwayat penulisan, tempat sampah, menghapus snapshot lama |
| **Dokumen** | Snapshot, mengganti nama, menduplikasi, memisahkan, menggabungkan, mengonversi, ikon, label, status, penanda |
| **Format** | Font (dengan pemasangan font), paragraf, gaya, daftar, sorot, menyalin pemformatan, mode revisi |
| **Jendela** | Catatan Sementara, jendela referensi cepat |
| **Bantuan** | **Temukan Perintah**, panduan ini, Panduan Mulai Cepat, proyek selamat datang, daftar semua placeholder, daftar pintasan papan ketik, bantuan untuk peta pikiran, submenu **Pengaya dan Pembaruan** (Pengelola Pengaya, Instal Pembaruan atau Pengaya, pemeriksaan pembaruan), menambahkan {{APP}} ke menu aplikasi (dan menghapusnya), submenu **Pemecahan Masalah dan Pengembangan** (informasi sistem, Komputer Ini, laporan crash, Buka Folder Plugin, panduan pengembang), Laporkan Masalah, lisensi pihak ketiga, Tentang |

**Temukan Perintah** (**Bantuan > Temukan Perintah…**) membuka jendela dengan kolom pencarian dan daftar semua perintah menu, masing-masing dengan menu dan pintasannya. Ketik sebagian nama (beberapa kata mempersempit pencarian, dalam urutan apa pun); **Return** atau klik dua kali menjalankan perintah yang dipilih. Perintah yang sedang nonaktif ditampilkan abu-abu dan tidak berjalan. Perintah ini tidak memiliki pintasan sendiri.

Submenu menampilkan nama singkat: **Berkas > Impor > Berkas**, **Dokumen > Pisahkan > Pada Pilihan**, **Navigasi > Buka > Sebagai Referensi Cepat**. Pintasan sebuah perintah ditampilkan di sebelahnya dan dapat diubah (bagian 19). Setiap entri menu memiliki hurufnya sendiri (bergaris bawah bila Anda menahan **Alt**), sehingga **Alt** dan huruf-huruf itu menjangkau setiap perintah. Sebuah perintah diakhiri dengan "…" hanya bila sebuah jendela terbuka lebih dulu dan menanyakan sesuatu sebelum perintah bekerja; perintah yang hanya menampilkan sesuatu (**Pratinjau Cetak**, **Statistik Proyek**, **Informasi Sistem**, dan sejenisnya) tidak memilikinya. Perintah yang tidak dapat bekerja ditampilkan abu-abu di menu; menu klik kanan sebuah item binder justru tidak menampilkan **Konversi ke Folder**, **Konversi ke Dokumen Teks**, **Konversi Lembar menjadi Objek**, dan **Gabungkan** bila perintah-perintah itu tidak dapat bekerja.

Tanpa proyek yang terbuka, menu **Sisipkan**, **Navigasi**, **Proyek**, **Dokumen**, dan **Format** berwarna abu-abu, demikian pula perintah menu lain yang memerlukan teks atau binder (Cari, Zoom, tata letak editor, Fokus ke Binder ...). Yang berfungsi tanpa proyek: membuat, membuka, dan mengimpor proyek, mengonversi berkas, memasang pembaruan dan pengaya, opsi, tema dan tata letak, alat menulis yang tidak memerlukan proyek, dan Bantuan.

### Panel menurut bentuk jendela

Jendela menata binder, editor, dan inspektor menurut bentuknya, dan berubah sendiri ketika bentuknya berubah (Anda menyeret jendela hingga lebih tinggi daripada lebarnya, atau memutar tablet). Jendela persegi dianggap lebar.

- **Jendela lebar** (lebih lebar daripada tingginya, atau persegi): tiga kolom. Pilih urutannya di **Tampilan > Tata Letak > Jendela lebar**: salah satu dari enam urutan binder, editor, dan inspektor. Lebarnya ditetapkan oleh garis di antara kolom.
- **Jendela tegak** (lebih tinggi daripada lebarnya): dua baris di bawah bilah alat. Satu baris adalah editor, selebar jendela; baris lainnya memuat binder dan inspektor berdampingan. Di bawah **Tampilan > Tata Letak > Jendela tegak**, **Editor di Tengah** menaruh baris editor di atas baris lainnya, **Editor di Bawah** di bawahnya; **Binder di Kiri** dan **Binder di Kanan** memilih sisi binder (inspektor mengambil sisi yang lain). Tinggi kedua baris serta lebar binder dan inspektor ditetapkan oleh garis di antara mereka. Saat papan ketik di layar tampil (Android), baris binder dan inspektor mempertahankan tingginya; geser garisnya bila Anda ingin yang berbeda.
- Binder atau inspektor yang disembunyikan menyerahkan ruangnya kepada yang lain di baris tegak, dalam lebar penuh; bila keduanya disembunyikan, editor memiliki seluruh jendela. Dua editor (**Tampilan > Pisahkan Editor**) tetap berada di dalam tempat editor.
- **Tampilan > Tata Letak > Atur Ulang Tata Letak** mengembalikan bentuk yang sedang dimiliki jendela ke nilai awalnya: binder, editor, inspektor di jendela lebar; editor di tengah dan binder di kiri di jendela tegak.

Kedua kelompok selalu ada di menu; masing-masing berlaku ketika jendela memiliki bentuk itu, dan setiap bentuk kembali seperti yang Anda tinggalkan (tempat dan ukuran). Pilihan itu milik komputer atau tablet ini dan disimpan di pengaturan program, tidak pernah di proyek: proyek yang Anda bagikan antarkomputer menyimpan penataan setiap perangkat, dan tidak ada apa pun tentangnya yang ditulis ke berkas proyek.

### Tata letak

Tata letak mengingat bagaimana jendela ditata: apakah binder, koleksi, inspektor, bilah alat, dan bilah format ditampilkan, selebar apa kolomnya, bagaimana editor dipisah, tampilan mana yang dipakai setiap editor, dan editor mana yang dimuat binder. Tata letak tidak menyangkut dokumen, sehingga setiap tata letak berfungsi di setiap proyek.

Pilih tata letak di bawah **Tampilan > Tata Letak** atau dari tombol **Tampilan** pada bilah alat. Tujuh tata letak sudah tersedia:

| Tata letak | Penataan |
|--------|-------------|
| **Default** | Binder, satu editor, inspektor |
| **Binder and Editor** | Binder dan teks, tanpa inspektor |
| **Editor Only** | Hanya teks dengan kedua bilah |
| **Outliner and Editor** | Kerangka di kiri, teks di kanan. Binder memuat grup ke dalam kerangka; klik pada sebuah baris membuka dokumen itu di kanan |
| **Corkboard and Editor** | Papan gabus di atas, teks di bawah, terhubung dengan cara yang sama |
| **Two Editors** | Dua teks berdampingan |
| **Planning** | Binder, papan gabus, dan inspektor dengan sinopsis dan catatan |

**Tampilan > Tata Letak > Simpan Tata Letak Saat Ini** menyimpan jendela seperti adanya sekarang dengan nama pilihan Anda; centang "Also remember the size of the window" bila tata letak harus mengubah ukuran jendela. **Kelola Tata Letak** menampilkan semua tata letak dengan gambar kecil dan memungkinkan Anda memakai, memperbarui (mengganti dengan jendela saat ini), mengganti nama, menghapus, mengekspor, dan mengimpor tata letak milik Anda. Tata letak bawaan tidak dapat diubah; simpan jendela yang sudah diubah dengan nama baru sebagai gantinya. Setiap tata letak dapat memiliki pintasan papan ketik (bagian 19, kelompok "Layouts").

### Antarmuka kecil

Untuk layar kecil, atau untuk menulis tanpa papan ketik, **antarmuka kecil** memberikan seluruh jendela kepada teks dan menaruh beberapa tombol bulat di tepinya: sebagai satu baris di bawah pada jendela yang tegak, sebagai satu kolom di tepi kanan pada jendela yang lebar. Nyalakan dan matikan dengan **Tampilan > Antarmuka Kecil** (item centang). Antarmuka ini mati saat program dimulai, dan hanya Anda yang menyalakannya; **Tampilan > Profil** menyimpannya seperti pengaturan lainnya, sehingga sebuah profil dapat dimulai dalam antarmuka kecil. Di Android item ini memiliki tiga pilihan: **Penuh**, **Kecil**, dan **Otomatis**; Otomatis menampilkan antarmuka kecil pada layar dengan kurang dari 1920 x 1080 piksel fisik (dipertahankan dalam kedua keadaan).

Tombol-tombolnya, dari yang pertama: **Menu** (setiap menu pada bilah menu, dan **Beralih ke Antarmuka Penuh**), **Teks Baru** (dokumen baru), **Tebal**, **Miring**, **Gaya** (gaya paragraf proyek), **Binder**, **Label dan Ikon** (label dan ikon dokumen yang dipilih), **Rekam Saja**, dan **Catatan**. Tombolnya bulat, memakai warna tema Anda, mengikuti ukuran pengaturan bilah alat Anda (Tampilan > Bilah Alat > Ukuran Bilah Alat, Besar di Android), dan tidak pernah lebih kecil dari 9 mm lebar dan tinggi di layar Anda. Papan ketik tetap berfungsi: tombol dapat dicapai dengan tombol Tab, pintasan perintah berfungsi seperti sebelumnya, dan garis di sekeliling bagian yang memegang papan ketik tetap ditampilkan.

- **Binder dan Catatan** terbuka sebagai panel yang menggeser masuk di atas teks: pada jendela yang lebar dari kiri (binder) dan dari kanan (catatan), dengan lebar yang cukup besar; pada jendela yang tegak sebagai lembar di atas seluruh area teks. Satu panel terbuka pada satu waktu. Panel binder adalah binder itu sendiri (koleksi, pencarian, tombol di bawahnya), panel catatan adalah inspektor dengan tab-tabnya, dibuka pada catatan dokumen. **Tutup**, **Esc**, atau tombol itu sekali lagi menutup panel; pada jendela yang tegak, memilih dokumen di lembar binder menutupnya, sehingga teks terlihat.
- **Rekam Saja** memulai dan menghentikan rekaman tanpa menulis teksnya (teks dapat ditulis nanti dengan **Transkripsikan** di daftar transkripsi). Tombol ini memerlukan pengaya **Transkripsi Ucapan**; tanpa pengaya itu, atau dengan versinya yang lebih lama, tombol ditampilkan abu-abu dan tooltipnya menyebutkan alasannya.
- Tidak ada tombol untuk snapshot, bilah alat disembunyikan, begitu pula bilah menu selama antarmuka kecil berjalan. **Tampilkan Binder**, **Tampilkan Inspektor**, **Tata Letak**, dan **Bilah Alat** di menu Tampilan ditampilkan abu-abu selama itu.

Mematikan antarmuka kecil mengembalikan antarmuka penuh seperti sebelumnya: bilah alat dan posisinya, bilah menu, lebar dan tinggi binder, editor, dan inspektor, serta apakah binder dan inspektor ditampilkan.

### Bilah alat dan tempatnya

Sejak 0.91 jendela memiliki delapan bilah alat dalam dua baris (sepuluh sejak 0.95, dengan dua yang awalnya mati). Baris pertama memuat **Utama** (Tampilan, teks baru, folder baru, tempat sampah, Urungkan, Ulangi), **Tema** (tombol tema, awalnya mati), **Profil** (profil yang dipakai dan menu untuk beralih, awalnya mati; sejak 0.95), **Cari** (tombol cari, kolom pencarian cepat, mode tampilan) dan **Alat** (penanda, Referensi Cepat, komentar, Kompilasi, Tampilkan Karakter Tak Terlihat, Mode Menulis, tombol alat menulis, inspektor). Baris kedua memuat **Gaya dan Font**, **Karakter dan Perataan**, **Paragraf dan Sisipkan**, **bilah ikon**, dan **Label** (satu tombol untuk setiap warna label proyek yang terbuka dan **Tanpa Label**; awalnya mati; sejak 0.95).

- **Memindahkan bilah alat.** Setiap bilah alat memiliki pegangan di bagian depannya (dua kolom titik; penunjuk berubah menjadi tanda silang dengan panah di atasnya). Seret pegangan, dan bilah alat pergi ke tempat yang Anda bawa:
  - **Sepanjang barisnya.** Selama penunjuk tetap berada di antara titik tengah kedua bilah alat tetangga, bilah alat mengikutinya seketika. Ruang kosong sebelum sebuah bilah alat selebar yang Anda buat. Saat bergerak ke kanan, bilah alat lebih dulu mengambil ruang kosong setelahnya; ketika mencapai bilah alat berikutnya, ia mendorongnya, sejauh bilah alat setelahnya dapat berdiri bersama di ujung baris. Saat bergerak ke kiri, ia berhenti di tempat bilah alat sebelumnya berakhir, dan bilah alat setelahnya tetap di tempatnya (seret ke arahnya bila Anda ingin menyatukannya lagi). Bilah alat pertama pada sebuah baris tetap di tepi jendela.
  - **Ke tempat lain dalam urutan.** Bawa penunjuk melewati titik tengah bilah alat tetangga, atau ke bilah alat lain mana pun di baris itu. Bilah alat tetap di tempatnya untuk sementara, dan garis berwarna sorot menunjukkan di mana ia akan berdiri; lepaskan tombol mouse untuk menaruhnya di sana. Jadi urutan apa pun dimungkinkan, juga untuk bilah alat pertama dan terakhir pada sebuah baris.
  - **Ke baris lain.** Bawa penunjuk ke baris yang lain: garis menunjukkan tempatnya di sana. Bilah alat apa pun dapat berdiri di baris mana pun, bilah alat format di samping bilah alat utama atau bilah alat pencarian di antara bilah alat format.
  - **Ke barisnya sendiri, atau ke tepi bawah.** Bawa penunjuk tepat di bawah baris bilah alat (garis melintang jendela muncul) untuk membuka baris baru, atau ke tepi bawah jendela untuk menaruh bilah alat di sana. Tepi bawah menerima bilah alat berdampingan dan dalam baris, seperti bagian atas.
  - **Esc**, atau pelepasan di tempat yang tidak menampilkan garis (misalnya di tengah jendela), membiarkan semuanya seperti semula.

  Bilah alat setelah tempat lama merapat, dan ruang kosong yang ada di depan bilah alat tetap berada di barisnya; di tempat barunya bilah alat berdiri langsung setelah bilah alat sebelumnya, dan Anda dapat menariknya lebih jauh ke kanan dari sana. Pada jendela yang terlalu sempit untuk ruang kosong yang Anda tinggalkan, ruang itu menyusut sebelum bilah alat kehilangan tombol; ruang itu kembali ketika jendela lebar lagi. Bilah alat yang dimatikan tetap menyimpan tempatnya. Bilah alat tidak dapat berdiri di sisi atau mengambang sebagai jendela tersendiri. {{APP}} mengingat baris, urutan, dan ruang kosongnya.
- **Mematikan bilah alat.** **Tampilan > Bilah Alat** memiliki sakelar untuk setiap bilah alat; daftar yang sama muncul dengan klik kanan pada bilah alat atau pada ruang kosong di samping bilah alat. **Tampilkan Bilah Alat** dan **Tampilkan Bilah Format** menyalakan atau mematikan satu baris utuh seperti sebelumnya (keempat bilah di baris pertama, ketiga bilah alat format), dan tata letak memakai kedua perintah ini; sakelar satu bilah tidak dapat dipakai selama barisnya dimatikan.
- **Tampilan > Bilah Alat > Atur Ulang Posisi Bilah Alat** mengembalikan semua bilah alat ke tempat standarnya (kedua baris dalam urutan standarnya, tidak ada apa pun di tepi bawah) dan menutup ruang di antara mereka.

### Bilah alat buatan Anda sendiri

**Tampilan > Bilah Alat > Sesuaikan Bilah Alat** (atau klik kanan pada bilah) membuka jendela dengan dua daftar: perintah yang tersedia di kiri, isi bilah di kanan. Pilih bilah alat di bagian atas.

- Tambahkan perintah dengan tombol panah atau klik dua kali. Setiap perintah menu tersedia, termasuk perintah alat menulis (Edit > Alat Menulis); kolom pencarian di atas daftar menemukannya.
- Hapus tombol dengan panah yang lain atau klik dua kali, dan pindahkan dengan tombol atas dan bawah atau dengan menyeretnya.
- **Pemisah** dan **Ruang Lentur** dapat dipakai sesering yang Anda mau. Ruang lentur mendorong apa yang mengikutinya ke kanan; dua ruang lentur memusatkan apa yang berdiri di antaranya.
- Beberapa entri lebih dari satu perintah: **Tampilan**, **Tambah**, **Cari**, **Kolom Pencarian Cepat**, **Mode Tampilan**, **Penanda Proyek**, **Komentar dan Catatan Kaki**, **Tata Letak**, dan **Tema** (tombol tema dengan menunya) untuk keempat bilah alat di baris pertama; daftar gaya, font, ukuran, dan jarak baris serta keempat tombol paragraf untuk ketiga bilah alat format. Kontrol seperti itu hanya ada satu: bila ditambahkan ke sebuah bilah alat, ia berpindah ke sana dari bilah alat yang memilikinya. Kontrol baris pertama tidak dapat masuk ke bilah alat format, begitu pula sebaliknya.
- Di bawah daftar Anda memilih ukuran semua bilah alat (Kecil, Normal, atau Besar; lihat di bawah) dan apakah bilah alat baris pertama menampilkan teks di samping atau di bawah ikon.
- **Atur Ulang** mengembalikan tombol-tombol bawaan program ke dalam bilah (tanpa bertanya; **Terapkan** atau **OK** membuatnya berlaku).

**Ukuran bilah alat.** **Tampilan > Bilah Alat > Ukuran Bilah Alat** (juga di menu klik kanan setiap bilah, dan di Sesuaikan Bilah Alat) menetapkan satu ukuran untuk semua bilah sekaligus: bilah alat utama, bilah format, bilah ikon, serta bilah Label dan Profil. **Kecil** memiliki ikon 16 piksel, **Normal** 22, dan **Besar** 32 (bawaan di tablet Android). Daftar dan kolom pada bilah (gaya, font, ukuran, jarak baris, pencarian cepat, tema) ikut membesar dan mengecil, sehingga bilah-bilah dalam satu baris tetap memiliki tinggi yang sama. Ukuran penskalaan tampilan sistem Anda berlaku di atasnya. Ukuran ini milik profil yang dipakai (lihat Profil) dan disimpan saat Anda keluar.

Jika sebuah bilah memuat lebih dari yang muat di jendela, panah di ujungnya membuka sisanya. Bilah alat yang tidak memiliki tombol sama sekali dimatikan; bila dinyalakan lagi di bawah **Tampilan > Bilah Alat**, ia memiliki set standarnya. Bilah alat yang Anda atur dalam versi sebelum 0.91 dipecah ke bilah alat yang baru: setiap tombol pergi ke bilah alat yang memuatnya dalam set standar, dan apa yang Anda lepas tetap terlepas.

### Bilah ikon

**Bilah ikon** memuat hingga sepuluh ikon dokumen yang sering Anda pakai. Klik pada salah satunya memberikan ikon itu kepada dokumen dan folder yang dipilih (di binder, atau di papan gabus atau kerangka tempat Anda bekerja); tombol **↺** setelahnya mengembalikan ikon standar mereka, dan tombol terakhir memilih ikon bilah. Tampilkan atau sembunyikan bilah dengan **Tampilan > Bilah Alat > Bilah Ikon**.

**Tampilan > Bilah Alat > Pilih Ikon untuk Bilah Ikon…** (juga di menu klik kanan bilah) membuka jendela dengan semua ikon di kiri, termasuk ikon proyek Anda sendiri, dan ikon bilah di kanan. Tambahkan dengan **Tambah →** atau klik dua kali, hapus dengan **← Hapus** atau klik dua kali, dan ubah urutan dengan **Naik** dan **Turun**. **Set Standar** mengembalikan sepuluh ikon bawaan program (Green Book, Proofread, Ticked, Unticked, Question Mark, Red Flag, Yellow Notepad, Character Sheet, Location Sheet, Warning).

---

## 3. Binder

Setiap proyek memiliki tiga folder khusus:

- **Draf** (di beberapa templat *Naskah*): semua yang akan menjadi buku Anda. Hanya isi Draf yang dikompilasi.
- **Riset**: catatan, lembar tokoh, gambar, PDF, dan apa pun yang Anda kumpulkan.
- **Tempat Sampah**: item yang dihapus menunggu di sini sampai Anda memilih **Proyek > Kosongkan Tempat Sampah**. Pertanyaannya, seperti setiap pertanyaan sebelum sesuatu yang tidak dapat diurungkan, menyebutkan langkahnya pada tombolnya dan memiliki **Batal** sebagai pilihan bawaan, sehingga tombol Enter tidak pernah menghapus apa pun (0.70).

### Bekerja dengan item

- **Dokumen baru**: **Ctrl+N** (atau tombol ➕). Dokumen ditambahkan di bawah item yang dipilih. Dokumen yang bernama "Untitled" (atau tanpa judul) memakai baris pertama teksnya sebagai judul begitu Anda menulis: paling banyak 60 karakter, dipotong pada batas kata, tanpa anotasi sebaris, catatan kaki sebaris, dan gambar. Judul mengikuti baris pertama sampai Anda mengganti nama dokumen sendiri; dokumen yang teksnya Anda hapus kembali menjadi "Untitled". Dokumen "Untitled" yang sudah memiliki teks mendapat namanya saat proyek dibuka. Judul itu adalah judul biasa: disimpan, dikompilasi (bila judul dikompilasi, baris pertama yang berupa judul muncul dua kali), dan ditulis ke proyek Scrivener.
- **Folder baru**: **Alt+Shift+N**.
- **Ganti nama**: **F2**, atau klik dua kali perlahan pada judul.
- **Pindahkan**: seret item dengan mouse, atau gunakan **Ctrl+Alt+tombol panah**: Atas dan Bawah memindahkan di dalam folder; Kiri memindahkan keluar dari folder; Kanan memindahkan masuk ke item di atasnya. Bila dijatuhkan *di antara* dua item, dokumen berada di sana; bila dijatuhkan *ke atas* sebuah item, dokumen menjadi subdokumennya. Jika Anda tidak sengaja menumpuk dokumen, matikan penjatuhan ke atas dokumen teks di **Berkas > Opsi > Perilaku** (folder selalu menerima penjatuhan).
- **Buka atau tutup seluruh cabang**: tahan **Alt** dan klik panah sebuah folder atau grup.
- **Hapus**: tekan **Delete**. Item masuk ke Tempat Sampah; papan gabus atau kerangka yang menampilkannya sebagai salah satu dari beberapa dokumen terpilih langsung melepaskannya (0.94.0).
- **Duplikat**: **Ctrl+D**.
- **Pisahkan dan gabungkan**: **Dokumen > Pisahkan > Pada Pilihan** (**Ctrl+K**) memotong dokumen pada kursor menjadi dua; ini tidak berfungsi di dalam tabel atau keterangan gambar (taruh kursor sebelum atau sesudahnya). **Dokumen > Gabungkan** (**Ctrl+Shift+M**) menyatukan dokumen yang dipilih ke dalam yang pertama, beserta sinopsis, kata kunci, dan catatannya; yang lain masuk ke Tempat Sampah.
- **Ubah ikon**: klik kanan > **Ubah Ikon** (atau **Dokumen > Ubah Ikon**), atau bilah ikon (bagian 2). Lihat "Ikon" di bawah.

**Klik kanan.** Pada sebuah dokumen, menu klik kanan memuat perintah untuk dokumen yang dipilih (ganti nama, duplikat, pindahkan ke Tempat Sampah, snapshot, label, status, dan lainnya). Pada ruang kosong di bawah dokumen, menu itu hanya menawarkan apa yang membuat sesuatu yang baru: **Teks Baru**, **Folder Baru**, **Baru dari Templat**, dan **Penampilan** binder.

Sebuah dokumen dapat memiliki subdokumen, sama seperti folder. Folder dan dokumen berperilaku sama; folder hanya tampil berbeda dan secara bawaan terbuka sebagai papan gabus.

**Tombol-tombol binder (0.94.0).** Dengan papan ketik di binder, **Return** membuka dokumen yang dipilih di editor dan memindahkan papan ketik ke sana (seperti pengelola berkas), dan tombol panah bergerak di sepanjang daftar. **Spasi** adalah bagian dari mengetik nama: ketik huruf-huruf pertama sebuah judul, termasuk spasinya ("Chapter t"), dan pilihan melompat ke judul pertama yang diawali demikian. Entri **Tampilan > Tambah Cepat Tombol di Binder** awalnya mati (juga setelah pembaruan). Bila dinyalakan, binder memiliki tombol tambah cepat: **Return** membuat dokumen kosong baru di bawah yang dipilih dan **Spasi** membuka dokumen yang dipilih sebagai panel Referensi Cepat (dan di daftar sebuah koleksi atau pencarian).

Klik di binder memuat dokumen ke editor. Setelah **Kembali**, tautan dokumen, atau dengan editor yang dikunci, editor dapat menampilkan dokumen lain daripada yang dipilih di binder: **bilah abu-abu** kemudian menandai dokumen editor di binder. **Navigasi > Tunjukkan di Binder** (**Ctrl+Shift+R**) memilihnya.

**Binder sempit, folder dalam.** Setiap tingkat folder menjorok 12 px (setengah dari sebelumnya; ubah di **Berkas > Opsi > Penampilan > Binder > Indentasi per tingkat**). Bila judul lebih panjang daripada lebar binder, bilah gulir muncul di bagian bawah binder, sehingga Anda dapat membaca ujung judul yang panjang tanpa melebarkan binder.

### Ikon

**Dokumen > Ubah Ikon** (juga di menu klik kanan binder, papan gabus, dan kerangka) dibuat seperti menu ikon Scrivener 3 untuk Windows:

- **Kembalikan Ikon ke Bawaan**: ikon standar sebuah dokumen, folder, atau berkas kembali.
- **Ikon dari Teks…**: membuat ikon dari paling banyak tiga karakter (misalnya *PR*, *Ch1*, atau *A*). Pilih warna kotak, huruf, dan batas; gambar kecil menunjukkan hasilnya. Ikon disimpan di proyek dan langsung diberikan kepada dokumen yang dipilih.
- **Kelola Ikon…**: ikon proyek ini. **Tambah Gambar…** mengubah gambar (PNG, SVG, JPEG, dan lainnya) menjadi ikon, **Dari Teks…** sama seperti di atas, **Ganti Nama…** dan **Hapus** mengubah daftar (dokumen dengan ikon yang dihapus mendapatkan kembali ikon standarnya), **Pakai untuk Dokumen Terpilih** memberikan ikon yang dipilih kepada dokumen yang dipilih. Ikon disimpan di proyek (folder *icons*), sehingga ikut bersamanya.
- **Terbaru**: lima ikon terakhir yang Anda pilih.
- **Book**, **Characters**, **Flag**, **Locations**, **Notes**, **To Do**, **TV**, **Way-Station**: submenu dengan ikon setiap kelompok dalam beberapa warna (misalnya Blue, Green, Red, dan Yellow Book; tujuh bendera; lima buku catatan; Ticked dan Unticked, Pass, Fail, dan Unclear di To Do).
- Lalu ikon tunggal: Back Matter, Blackboard, Calendar, Clapper Board, Clock, Cog, Eye, Figures and Graphs, Filing Cabinet, Front Matter, Information, Lectern, Light Bulb, Magnifying Glass, Music, Musical Notes, Outline, Pencil, Proofread, Question Mark, Reviewer Comments, Scissors, Speech Bubble, Speech Marks, Statistics, Sync Folder, Tables, Test Tube, Thought Bubble, dan Warning.
- **Ikon Proyek**: ikon Anda sendiri (bila proyek memilikinya), dan **Ikon Lainnya (Desktop)**: ikon tema desktop yang ditawarkan versi-versi sebelumnya.

**Warna label pada ikon.** Bagian putih pada banyak ikon (halaman buku catatan, kain bendera putih, kertas lembar, kotak ikon yang dibuat dari teks) mengambil warna label dokumen. Nyalakan atau matikan dengan **Tampilan > Gunakan Warna Label di > Ikon** (atau **Berkas > Opsi > Penampilan > Binder**).

**Ikon dan Scrivener.** Ikon {{APP}} digambar baru, tetapi setiap ikon membawa nama ikon Scrivener yang diwakilinya. Proyek dalam format Scrivener (bagian 14) mempertahankan nama-nama ini, sehingga Scrivener menampilkan ikonnya sendiri untuk nama itu; ikon yang dibuat di Scrivener kembali ke sini sebagai ikon yang sesuai. Ikon Anda sendiri masuk ke folder *Icons* pada *.scriv*. Ikon yang tidak memiliki gambar di program ini mempertahankan namanya dan ditampilkan sebagai ikon terdekat di sini.

### Label, status, dan "Sertakan dalam Kompilasi"

Klik kanan sebuah item untuk menetapkan **label**-nya (warna, misalnya untuk tokoh sudut pandang) dan **status**-nya (misalnya *First Draft* atau *Done*). Ubah daftarnya di **Proyek > Pengaturan Proyek**.

Dokumen atau folder yang memiliki label juga menampilkan warnanya di inspektor: kartu sinopsis dan catatan dokumen mendapat rona terang warna label (15 % dari kejenuhannya), sehingga Anda langsung melihat label apa yang dimiliki item itu. Item tanpa label, atau dengan label abu-abu, mempertahankan warna kartu yang normal.

Bilah alat **Label** (**Tampilan > Bilah Alat > Label**, awalnya mati) menampilkan semua warna label proyek yang terbuka berdampingan, dengan satu tombol untuk tanpa label. Satu klik langsung menetapkan label dokumen yang dipilih, seperti **Dokumen > Label**; tooltip setiap warna menyebutkan labelnya. Label yang dipakai proyek baru di awal ditetapkan di **Berkas > Opsi > Penampilan > Warna Standar**.

Item dengan **Sertakan dalam Kompilasi** yang dimatikan tetap berada di Draf tetapi tidak diikutkan saat Anda mengompilasi.

### Koleksi

Koleksi adalah daftar dokumen tambahan, misalnya "Needs research" atau "Scenes with Anna". Tampilkan dengan tombol **Koleksi**. Gunakan tombol ➕ di atas binder untuk membuat koleksi dari dokumen yang dipilih, atau klik kanan dokumen dan pilih **Tambahkan ke Koleksi**. Mengeluarkan dokumen dari koleksi tidak menghapusnya. Pencarian tersimpan (bagian 9) juga merupakan koleksi; koleksi itu memperbarui dirinya sendiri.

- **Tambahkan dokumen** dengan menyeretnya dari binder (atau dari ikon header editor, papan gabus, kerangka) ke tab koleksi.
- **Di dalam daftar** sebuah koleksi, seret dokumen ke urutan yang Anda sukai (urutan binder tidak berubah), jatuhkan dokumen di suatu tempat, dan tekan **Delete** untuk mengeluarkan dokumen yang dipilih dari koleksi. Koleksi pencarian tidak dapat diubah dengan cara ini.
- **Seret tab** untuk menaruh koleksi dalam urutan lain.
- Dokumen di dalam daftar dapat diseret seperti dokumen di binder, dan **Spasi** membuka yang dipilih sebagai panel Referensi Cepat.

---

## 4. Menulis

### Pemformatan teks

| Tindakan | Pintasan |
|--------|----------|
| Tebal / miring / garis bawah | Ctrl+B / Ctrl+I / Ctrl+U |
| Sorot | Ctrl+Shift+H |
| Superskrip / subskrip | Ctrl+Shift++ / Ctrl+= (tata letak Jerman: Ctrl+Shift+D untuk subskrip, lihat "Pintasan dan tata letak papan ketik" di bawah) |
| Rata kiri / tengah / kanan | Ctrl+{ / Ctrl+\| / Ctrl+} (tata letak Jerman: Alt+Shift+L / Alt+Shift+E / Alt+Shift+R) |
| Jendela font | Ctrl+T |
| Ukuran font lebih besar / lebih kecil | Ctrl+> / Ctrl+< |
| Jendela paragraf | Ctrl+Alt+P |
| Salin pemformatan / tempel pemformatan | Ctrl+Alt+C / Ctrl+Alt+V |
| Perbesar / perkecil / atur ulang zoom | Ctrl++ / Ctrl+- / Ctrl+0 |
| Zoom area di bawah mouse | Ctrl+roda mouse |
| Tampilkan atau sembunyikan penggaris | Ctrl+Alt+R |
| Tempel tanpa pemformatan | Ctrl+Shift+V |
| Tempel dengan pemformatan (dari program lain) | Ctrl+Alt+Shift+V |
| Urungkan / ulangi | Ctrl+Z / Ctrl+Shift+Z (lihat "Urungkan dan ulangi" di bawah) |

Tebal, miring, superskrip, dan sejenisnya bekerja pada pilihan. Tanpa pilihan, di dalam sebuah kata perintah itu mengubah seluruh kata; di akhir kata (atau bila tidak ada apa pun di sekitar kursor) perintah itu mengubah apa yang Anda ketik berikutnya: ketik "H", tekan Ctrl+=, ketik "2", tekan Ctrl+= lagi dan ketik "O" untuk H₂O.

**Gaya** (Heading 1–6, Title, Subtitle, Block Quote, dan gaya Anda sendiri) dipilih dari daftar di sisi kiri bilah format. Gaya memberi tahu Kompilasi apa itu sebuah paragraf, jadi pakailah gaya sebagai ganti membuat teks besar dan tebal secara manual. Lihat "Gaya" di bawah.

**Apa yang ditampilkan bilah format.** Bilah menampilkan teks seperti yang Anda lihat, untuk seluruh pilihan. **Tebal**, **Miring**, **Garis bawah**, **Coret**, superskrip, dan subskrip menyala bila semua teks yang dipilih tampak demikian, juga bila gayanya yang membuatnya begitu (tebalnya sebuah judul). Klik mengubah apa yang Anda lihat: **Tebal** pada judul yang tebal membuatnya tidak tebal, dan tetap tidak tebal ketika proyek disimpan dan dibuka lagi. Font, ukuran, jarak baris, tombol perataan, dan daftar gaya juga menampilkan pilihan; bila pilihan memuat dua font, dua ukuran, dua perataan, atau paragraf dengan dua gaya, kolom itu tetap kosong (tidak ada tombol perataan yang menyala). Ukuran yang diketik ke kolom kosong memberikan ukuran itu kepada seluruh pilihan.

**Daftar**: gunakan tombol daftar, atau ketik "- " atau "1. " di awal paragraf.

**Tipografi cerdas**: tanda kutip lurus menjadi tanda kutip melengkung, dan dua tanda hubung menjadi tanda pisah em saat Anda mengetik. Matikan ini di **Berkas > Opsi > Pengeditan**.

**Mode revisi** (**Format > Mode Revisi**): semua yang Anda ketik diberi warna salah satu dari lima warna revisi, sehingga Anda dapat melihat apa yang berubah dalam satu putaran penyuntingan. Pilih **Mati** untuk berhenti.

**Spasi dan indentasi di bilah format.** Empat tombol setelah tombol daftar menetapkan paragraf yang memuat kursor atau pilihan: **spasi sebelum** dan **spasi sesudah** paragraf (0, 3, 6, 9, 12, atau 18 pt), **indentasi baris pertama**, dan **indentasi seluruh paragraf** (dalam sentimeter atau inci, seperti yang ditampilkan penggaris). Nilai paragraf pada kursor dicentang; **Lainnya...** menanyakan nilai apa pun. **Edit > Urungkan** membatalkan perubahan. Bilah format yang Anda atur sendiri mendapatkan keempat tombol itu satu kali, setelah tombol daftar; **Tampilan > Bilah Alat > Sesuaikan Bilah Alat** memindahkan atau menghapusnya.

**Urungkan dan ulangi.** **Edit > Urungkan** dan **Edit > Ulangi** bekerja di kolom teks yang memegang papan ketik (editor, sinopsis, catatan, judul) dan menyebutkan apa yang akan dibatalkan: "Urungkan Pengetikan", "Urungkan Tempel", "Urungkan Tebal", "Ulangi Hapus". Dengan papan ketik di tempat lain, misalnya di binder, keduanya berwarna abu-abu: memindahkan dokumen ke Tempat Sampah, mengosongkan Tempat Sampah, dan menghapus snapshot tidak dapat diurungkan dengan cara ini (dokumen di Tempat Sampah kembali dengan menyeretnya keluar; lihat bagian 11 untuk snapshot). Panah di samping tombol **Urungkan** dan **Ulangi** pada bilah alat mencantumkan langkah-langkahnya, yang berikutnya lebih dulu; memilih salah satu membatalkan (atau mengembalikan) langkah itu dan setiap langkah sebelumnya dalam daftar. Setiap perintah menu Format adalah satu langkah.

**Warna teks dan sorot di bilah format.** Kedua tombol dengan garis berwarna (pena untuk menyorot, huruf *T* untuk warna teks) bekerja seperti di Scrivener dan Word: klik menerapkan warna yang ditampilkan di garis itu pada teks yang dipilih, atau, tanpa pilihan, pada apa yang Anda ketik berikutnya. Biarkan mouse di atas tombol sejenak dan jendela kecil dengan **merah, biru, kuning, hijau, putih, dan hitam** terbuka di bawahnya, ditambah **Tanpa Sorot** (atau **Otomatis** untuk warna teks) dan **…** untuk warna lain mana pun. Klik pada sebuah warna menerapkannya dan menjadikannya warna tombol, yang menyimpannya untuk klik berikutnya (juga setelah dimulai ulang). Sorotan digambar persis dengan warna yang Anda pilih, baik di halaman terang maupun gelap. Bila teks di atasnya akan sulit dibaca (kontras di bawah 4,5 : 1), teks ditampilkan gelap atau terang hanya di layar; warna yang tersimpan, dan sorotan di berkas yang disimpan dan dikompilasi, tetap seperti yang dipilih.

### Font

Semua teks dalam sebuah proyek memiliki satu tampilan: **pemformatan bawaan** (lihat di bawah). Di atasnya Anda dapat memberi potongan teks mana pun font tersendiri.

- **Bilah format**: daftar font dan kolom ukuran mengubah teks yang dipilih seketika. Tanpa pilihan, keduanya mengubah **seluruh paragraf** yang memuat kursor (dan apa yang Anda ketik berikutnya di sana). Panah kolom ukuran mengubah teks pada setiap klik; ukuran yang Anda ketik berlaku dengan **Enter**. Bilah format, menu Format, dan pintasannya bekerja pada teks tempat kursor terakhir berada: editor, catatan di inspektor, teks penanda di inspektor, atau kolom teks panjang sebuah objek (di sana: tebal, miring, garis bawah, coret, warna teks, dan sorot). Klik di kolom ukuran tidak mengubahnya.
- Setelah perubahan di bilah format, bilah di atas editor menawarkan hal yang sama untuk seluruh proyek: pada paragraf dengan gaya (misalnya judul) **Ubah Gaya “Heading 1”** memberikan font atau ukuran baru kepada gaya itu, dan semua teks bergaya itu di setiap dokumen mengikutinya; pada paragraf tanpa gaya **Pakai untuk Semua Teks** menjadikannya font atau ukuran proyek (lihat "Pemformatan bawaan" di bawah). Klik **×**, atau lanjutkan saja menulis di tempat lain, untuk membiarkan perubahan di tempatnya. Setiap dokumen yang berubah mendapat snapshot lebih dulu.
- **Format > Font > Tampilkan Font…** (**Ctrl+T**) membuka jendela font: font, jenis huruf (tipis, reguler, tebal, miring, dan apa pun yang ditawarkan font), ukuran, garis bawah, coret, huruf kapital (kapital kecil atau semua kapital), jarak huruf dalam persen, dan warna teks. Hanya apa yang Anda ubah di jendela yang diterapkan, sehingga pilihan dengan kata tebal dan biasa mempertahankannya. **Font Proyek** mengembalikan teks ke font, ukuran, dan warna proyek.
- **Tebal, Miring, Garis bawah, Coret**: tombol menyala bila *semua* teks yang dipilih memiliki format itu, bagaimanapun Anda memilihnya (juga dari kanan ke kiri). Pilihan yang hanya sebagian tebal menampilkan Tebal mati, dan **Tebal** kemudian membuat semuanya tebal; klik kedua membuat semuanya biasa. Di jendela font, jenis huruf yang Anda pilih juga ditetapkan untuk seluruh pilihan.
- **Format > Font > Lebih Besar / Lebih Kecil** (**Ctrl+>** / **Ctrl+<**) mengubah ukuran satu poin; setiap bagian pilihan mempertahankan ukurannya sendiri dan membesar atau mengecil.
- **Format > Font > Warna Teks…** dan **Warna Teks Otomatis**.

Memilih font atau ukuran proyek sendiri disimpan sebagai "tanpa font tersendiri": teks seperti itu mengikuti proyek ketika Anda mengubah pemformatan bawaan nanti.

**Font dengan beberapa desain.** Beberapa font menyimpan beberapa desain satu jenis huruf di bawah satu nama: Sitka milik Windows 11, misalnya, memuat ukuran optis Small, Text, Subheading, Heading, Display, dan Banner (masing-masing digambar untuk rentang ukurannya sendiri, yang lebih besar lebih sempit dan dengan huruf kecil yang lebih kecil), dan beberapa font memuat versi ringkas atau lebar dengan cara yang sama. {{APP}} mendaftar setiap desain sebagai font tersendiri (misalnya **Sitka Small**, **Sitka Text**, **Sitka Banner**), dengan hanya jenis huruf desain itu, sehingga tebal dan miring tetap berada dalam desain yang Anda pilih; sebelum 0.40.1, teks tebal dapat muncul dalam desain lain dan tampak lebih kecil dan lebih sempit. Berkas font tidak diubah. Font dengan nama asli mempertahankan desainnya sendiri, sehingga teks yang sebelumnya diatur dengannya mempertahankan tampilannya.

**Memasang font.** **Format > Font** memiliki perintah untuk font yang belum ada di komputer Anda:

- **Instal Berkas Font…**: pilih berkas font (TTF, OTF, TTC, PFB, WOFF, atau berkas ZIP yang memuatnya). Berkas disalin ke folder font Anda sendiri (*~/.local/share/fonts*), dan daftar font {{APP}} langsung mengenalnya; program lain melihatnya setelah dimulai berikutnya. Tidak perlu kata sandi, dan font hanya untuk pengguna Anda.
- **Buka Folder Font**: membuka folder itu di pengelola berkas; font yang Anda taruh sendiri di sana ditemukan setelah **Muat Ulang Daftar Font**.
- **Buka Pengelola Font Desktop**: membuka pengelola font KDE (System Settings > Text & Fonts > Font Management) atau program font desktop lain, tempat Anda juga dapat memasang font untuk semua pengguna dan menghapus font.
- **Muat Ulang Daftar Font**: mencari font baru dan menamainya.

### Pemformatan paragraf

**Format > Paragraf > Paragraf…** (**Ctrl+Alt+P**) menetapkan, untuk paragraf yang memuat kursor atau pilihan: perataan, indentasi kiri dan kanan, indentasi baris pertama (nilai negatif menghasilkan indentasi menggantung), spasi sebelum dan sesudah, jarak baris (kelipatan tinggi baris, tinggi pasti, atau tinggi minimum), dan perhentian tab (kiri, tengah, kanan, titik desimal). Hanya apa yang Anda ubah yang diterapkan. **Atur Ulang ke Bawaan** menghapus pemformatan yang Anda tetapkan secara manual; paragraf kembali mengikuti gayanya dan proyek.

Cara cepat: tombol perataan, daftar jarak baris di bilah format, **Format > Paragraf > Jarak Baris**, dan **Tambah Indentasi / Kurangi Indentasi**.

Menerapkan gaya paragraf menggantikan pemformatan manual pada paragraf yang dijangkaunya: indentasi, spasi, perataan, dan perhentian tab, serta font, ukuran, warna, jarak huruf, dan kapital teks di dalamnya. Tebal, miring, garis bawah, coret, sorotan, dan gaya karakter tetap. Pilih teks lintas beberapa paragraf untuk memberikan gaya kepada semuanya, seluruh setiap paragraf; pilihan yang berakhir tepat di awal paragraf (misalnya setelah **Shift+Bawah**) membiarkan paragraf itu.

**Format > Salin Pemformatan** (**Ctrl+Alt+C**) mengingat pemformatan font dan paragraf pada kursor; **Terapkan Pemformatan yang Disalin** (**Ctrl+Alt+V**) menerapkannya pada pilihan, atau pada seluruh paragraf bila tidak ada yang dipilih.

### Karakter tak terlihat

**Tampilan > Pengeditan Teks > Tampilkan Karakter Tak Terlihat** (juga tombol ¶ pada bilah alat) menampilkan karakter yang tidak Anda lihat: titik untuk setiap spasi, panah untuk tab, lingkaran kecil untuk spasi tanpa jeda, panah bengkok untuk pemisah baris (**Shift+Enter**) dan ¶ di akhir setiap paragraf. Semuanya hanya digambar di layar, di teks utama setiap editor; tidak pernah menjadi bagian teks, cetakan, atau kompilasi. Pilihannya disimpan. Warnanya adalah warna teks, dipudarkan ke warna kertas; warna tersendiri ditetapkan di **Berkas > Opsi > Penampilan > Tanda Tekstual** (atau klik kanan teks > **Penampilan**), dan menjadi bagian tampilan proyek seperti kertas dan teks (mode menulis mempertahankan warna teks yang dipudarkan).

### Penggaris

**Tampilan > Pengeditan Teks > Tampilkan Penggaris** (**Ctrl+Alt+R**) menampilkan penggaris di atas teks. Penggaris itu milik paragraf yang memuat kursor. **Tampilkan Bilah Format** ada di menu yang sama, tepat sesudahnya (0.99.0), dan di **Tampilan > Bilah Alat**.

- Penanda di atas adalah **indentasi baris pertama**, penanda di bawah adalah **indentasi kiri** dan **indentasi kanan**. Seretlah. Indentasi kiri membawa baris pertama bersamanya.
- **Klik** di bagian bawah penggaris untuk menambah perhentian tab. Seret perhentian tab untuk memindahkannya; seret ke atas atau ke bawah keluar dari penggaris untuk menghapusnya. **Klik kanan** perhentian tab untuk menjadikannya kiri, tengah, kanan, atau desimal.
- Penanda menempel pada seperempat sentimeter (seperenam belas inci); tahan **Shift** untuk menaruhnya dengan bebas.
- **Klik dua kali** penggaris untuk jendela paragraf. Menu klik kanan beralih antara sentimeter dan inci (juga di **Berkas > Opsi > Pengeditan**).

Perhentian tab diukur dari awal baris pertama paragraf. Tanpa perhentian tab sendiri, paragraf memiliki posisi tab setiap 2,1 cm.

Penggaris menampilkan panjang yang sebenarnya. Zoom memperbesar huruf di layar tetapi tidak indentasi dan perhentian tab, jadi bandingkan jarak pada 100 %.

### Pemformatan bawaan

- **Proyek ini**: **Proyek > Pengaturan Proyek > Pemformatan** (juga tombol penggeser di bilah format) menetapkan font, ukuran, jarak baris, indentasi baris pertama, spasi setelah paragraf, dan perataan teks seluruh proyek (empat tombol: kiri, tengah, kanan, rata kiri-kanan). Di Panel Gaya, **No Style** > **Definisikan Ulang dari Pilihan** mengambil nilai-nilai ini dari paragraf pada kursor (lihat "Gaya" di bawah). **Terapkan** mengubah semua dokumen sekaligus dan membiarkan jendela terbuka, sehingga Anda dapat melihat hasilnya di belakangnya; **OK** melakukan hal yang sama dan menutup jendela. **Pakai Saat Ini** mengambil nilai dari paragraf yang memuat kursor.
  - Dengan **Ubah juga teks yang memiliki font, ukuran, atau spasi tersendiri** (aktif secara bawaan), teks yang diformat secara manual atau yang datang dengan font sendiri dari impor ikut berubah, selama memiliki font, ukuran, dan spasi utama dokumennya. Apa yang berbeda dari teks utama tetap seperti adanya: baris judul yang lebih besar, bagian dengan font lain, paragraf tanpa indentasi. Matikan kotak itu untuk membiarkan semua teks yang diformat secara manual.
  - Setiap dokumen yang berubah mendapat snapshot lebih dulu (inspektor, tab snapshot, "Before the project formatting changed"), sehingga Anda dapat kembali. Dokumen skenario mempertahankan format sendiri.
- **Proyek baru**: **Berkas > Opsi > Pengeditan > Pemformatan proyek baru**, dengan keempat tombol perataan yang sama dan **Pakai Pemformatan di Editor Saat Ini**. Proyek yang sudah ada mempertahankan pemformatannya. Standarnya adalah 11,0 pt, jarak baris 1,09, tanpa indentasi baris pertama, dan 6 pt setelah paragraf. Proyek yang diimpor dari Scrivener juga dimulai dengan nilai-nilai ini (dengan font yang paling banyak dipakai di proyek Scrivener).
- **Kembali ke bawaan**: pilih dokumen di binder dan pilih **Dokumen > Konversi > Teks ke Pemformatan Bawaan…**. Anda menentukan apakah font dan warna, indentasi dan spasi, perataan, dan perhentian tab dihapus. Tebal, miring, gaya, catatan, dan tautan selalu tetap. Snapshot setiap dokumen dibuat lebih dulu (inspektor, tab snapshot), sehingga Anda dapat kembali.

### Gaya

Gaya menyimpan pemformatan dengan sebuah nama. Teks mengingat nama itu, sehingga ketika Anda mengubah gaya, semua teks yang memakainya ikut berubah, di seluruh proyek.

Ada dua jenis:

- **Gaya paragraf** berlaku untuk seluruh paragraf. Gaya ini memuat pemformatan paragraf (perataan, indentasi, spasi, jarak baris, perhentian tab) dan, bila Anda mau, pemformatan karakter untuk paragraf (font, ukuran, tebal, miring, warna). Dalam daftar, gaya ini ditandai "¶", atau "¶a" bila menyertakan pemformatan karakter.
- **Gaya karakter** berlaku untuk teks yang dipilih, atau kata pada kursor. Gaya ini hanya memuat pemformatan karakter. Ditandai "a".

Biarkan teks isi pada **No Style**. Teks itu mengikuti pemformatan bawaan proyek, dan Kompilasi dapat memberinya tampilan apa pun. Gunakan gaya hanya untuk teks yang harus tampak berbeda dari teks isi.

**Menerapkan gaya**: pilih di daftar sebelah kiri bilah format, di bawah **Format > Gaya**, di menu klik kanan pada teks, atau di **Panel Gaya** (**Format > Gaya > Panel Gaya**), jendela kecil yang tetap terbuka di samping teks; klik dua kali pada sebuah baris menerapkan gaya itu. **No Style** menghapus gaya karakter dari pilihan; bila tidak ada, menghapus gaya paragraf. Jendela dan pertanyaan yang dibuka Panel Gaya muncul di atas panel.

**Daftar gaya** pada bilah format, menu klik kanan (**Gaya**), dan **Format > Gaya** (0.99.0; dengan lebih dari sepuluh gaya berupa daftar yang dapat digulir, seperti di bilah format) menampilkan gaya yang Anda pilih, dalam urutan yang Anda pilih: **Sesuaikan Daftar Gaya…** (di bagian bawah daftar, dan di bawah **Format > Gaya**) mencantumkan semua entri; hapus centang pada yang tidak Anda perlukan, seret ke urutan lain atau gunakan panah, dan **Semua, dalam Urutan Gaya** mengembalikan daftar penuh. Dengan lebih dari sepuluh entri, daftar menampilkan sepuluh dan dapat digulir. Di bawahnya, selalu terlihat: **Gaya Baru dari Pilihan…**, **Definisikan Ulang Gaya dari Pilihan…**, **Panel Gaya**, dan **Sesuaikan Daftar Gaya…**. Gaya yang tidak ada di daftar tetap disebut namanya di bilah format bila teks memakainya; **Format > Gaya** selalu mencantumkan semua gaya. Pilihan itu milik proyek (gaya baru muncul di akhir daftar) dan menjadi bagian gaya dalam berkas pengaturan proyek.

**No Style adalah gaya teks utama.** Di Panel Gaya, klik kanan **No Style** (atau pilih dan gunakan tombol Definisikan Ulang) dan pilih **Definisikan Ulang dari Pilihan…**: font, ukuran, jarak baris, indentasi baris pertama, spasi setelah paragraf, dan perataan paragraf pada kursor menjadi gaya teks utama proyek (nilai **Proyek > Pengaturan Proyek > Pemformatan**). Sebuah pertanyaan menampilkan nilainya lebih dulu. Semua teks tanpa gaya di seluruh proyek mengikutinya, juga teks dengan font, ukuran, atau spasi tersendiri; teks bergaya mempertahankan apa yang ditetapkan gayanya. Setiap dokumen yang berubah mendapat snapshot lebih dulu.

**Membuat gaya**:

1. Format beberapa teks sesuai tampilan yang diinginkan untuk gaya itu, lalu klik di dalamnya atau pilih sebagian.
2. Pilih **Format > Gaya > Gaya Baru dari Pilihan…** (atau tombol plus Panel Gaya).
3. Masukkan nama dan pilih apa yang disimpan: semua pemformatan (gaya paragraf dengan pemformatan karakter), hanya pemformatan paragraf, atau atribut karakter (gaya karakter).
4. Tentukan apakah font dan ukuran font menjadi bagian gaya. Tanpa keduanya, gaya mempertahankan font teks di sekitarnya.
5. Untuk gaya paragraf, pilih gaya apa yang didapat paragraf berikutnya ketika Anda menekan Enter di akhir.

Gaya paragraf hanya menyimpan apa yang berbeda dari teks isi. Semua yang lain tetap mengikuti pemformatan proyek, sehingga font bawaan baru juga menjangkau teks bergaya.

**Mengubah gaya**:

- **Format > Gaya > Definisikan Ulang Gaya dari Pilihan…** mengambil pemformatan teks pada kursor, seperti di atas. Gaya paragraf tetap gaya paragraf, gaya karakter tetap gaya karakter.
- **Format > Gaya > Edit Gaya** (atau tombol pensil Panel Gaya) membuka jendela dengan nama, **Font…**, **Paragraf…**, dan gaya paragraf berikutnya. Pratinjau menampilkan hasilnya.

Setelah itu {{APP}} menelusuri seluruh proyek dan memperbarui teks, di dokumen yang terbuka dan di berkas dokumen lainnya. Apa yang disentuh perubahan (font, ukuran, tebal, miring, warna, jarak huruf, kapital, perataan, indentasi, spasi, jarak baris, perhentian tab) menjangkau **semua** teks gaya itu, juga di tempat teks itu memiliki nilai sendiri: ditetapkan manual, atau dibawa oleh impor (judul hasil impor biasanya menyebutkan font, ukuran, dan spasinya di setiap paragraf). Pemformatan yang tidak disentuh perubahan tetap seperti adanya, misalnya kata yang diberi warna secara manual di sebuah judul ketika hanya ukuran judul yang berubah. Setiap dokumen yang berubah mendapat snapshot lebih dulu ("Before the style ... changed"). Ini tidak dapat diurungkan dengan **Edit > Urungkan**; pulihkan snapshot, definisikan ulang gaya lagi, atau gunakan **Kembalikan ke Definisi Bawaan** di Panel Gaya untuk gaya bawaan.

**Mengganti nama dan menghapus**: ganti nama gaya di **Edit Gaya**. Heading, Title, Subtitle, Block Quote, Centered, dan Caption mempertahankan namanya, karena Kompilasi dan berkas Markdown mengenalnya dari nama. Bila Anda menghapus gaya (**Format > Gaya > Hapus Gaya**), teks yang memakainya mempertahankan tampilannya sebagai pemformatan manual.

**Proyek lain**: **Format > Gaya > Impor Gaya…** membaca gaya dari proyek lain. **Pakai Gaya Ini di Proyek Baru** menjadikan gaya proyek yang terbuka sebagai set awal setiap proyek baru; **Berkas > Opsi > Pengeditan** kembali ke gaya bawaan.

Di Panel Gaya, tombol panah mengubah urutan gaya di semua daftar.

### Markdown atau RTF: cara dokumen disimpan

Setiap proyek menyimpan dokumennya dalam salah satu dari dua format. Anda memilihnya saat membuat atau mengimpor proyek, dan dapat berganti nanti di **Proyek > Pengaturan Proyek > Pemformatan**.

- **Markdown** (disarankan): berkas teks biasa yang dapat dibuka program apa pun, sekarang dan lima puluh tahun lagi. Tampilan teks berasal dari pemformatan bawaan. Font dan pemformatan paragraf yang Anda tetapkan secara manual ditulis ke dalam teks sebagai penanda kecil, hanya di tempat Anda memakainya (bagian 18); semua yang lain tetap polos.
- **RTF**: berkas teks kaya seperti pengolah kata. Setiap paragraf membawa pemformatan lengkapnya, sehingga perubahan pemformatan bawaan tidak menjangkau teks yang sudah ada.

Berganti format mengonversi setiap dokumen. Dari RTF ke Markdown, font, warna, perataan, indentasi kiri dan kanan, dan perhentian tab dipertahankan. Jarak baris, indentasi baris pertama, dan spasi paragraf pada teks yang berasal dari program lain kemudian mengikuti proyek. Buat cadangan lebih dulu (**Berkas > Cadangkan > Cadangkan Sekarang**).

Teks yang ditempel dari program lain ke proyek Markdown mengambil tampilan teks di sekitarnya (tebal, miring, dan tautan tetap). Teks yang disalin di dalam {{APP}} mempertahankan fontnya. **Edit > Tempel dengan Pemformatan** (**Ctrl+Alt+Shift+V**) juga mempertahankan tampilan teks dari pengolah kata atau halaman web: font, ukuran, warna, serta perataan, indentasi, dan spasi paragrafnya (paragraf pertama yang ditempel membawa pemformatan paragrafnya hanya ke dalam paragraf kosong). **Edit > Tempel dan Samakan Gaya** (**Ctrl+Shift+V**) melakukan sebaliknya dan menempel teks biasa. Menu klik kanan pada teks menawarkan ketiganya, satu di bawah yang lain: **Tempel**, **Tempel dan Samakan Gaya**, dan **Tempel dengan Pemformatan**.

Untuk mengonversi satu berkas di luar proyek, gunakan **Berkas > Konversi Markdown ⇄ Berkas RTF…**.

### Komentar dan catatan kaki

- **Komentar** (**Shift+F2**): pilih beberapa teks dan tambahkan komentar. Teks disorot dan komentar muncul di inspektor.
- **Catatan kaki** (**Ctrl+Shift+F2**): pilih kata yang menjadi tempat catatan kaki; tulis catatan kaki di inspektor.
- **Anotasi sebaris** (**Ctrl+Shift+A**): catatan yang ditulis langsung di dalam teks, ditampilkan berwarna dalam gelembung. Anotasi dihapus saat Anda mengompilasi (kecuali Anda memilih mengekspornya sebagai komentar).
- **Catatan kaki sebaris** (**Ctrl+Alt+F**): catatan kaki yang ditulis langsung di dalam teks.

Untuk mengubah warna anotasi sebaris, pilih anotasi itu (**Edit > Pilih Anotasi atau Catatan** memilih seluruh catatan pada kursor) dan pilih warna teks; anotasi mempertahankan warna itu. **Format > Font > Warna Teks Otomatis** mengembalikan warna merah yang biasa.

Klik komentar di inspektor untuk melompat ke sana. Klik kanan untuk mengubah warnanya, mengonversinya ke jenis catatan lain, atau menghapusnya.

Klik pada teks yang berkomentar membuka inspektor dengan komentar itu. **Alt+klik** sebaliknya membuka komentar atau catatan kaki di jendela kecil pada mouse, tempat Anda dapat membaca dan mengubahnya tanpa inspektor; klik di tempat lain atau tekan **Esc** untuk menutupnya.

### Gambar dan tautan

- **Sisipkan > Gambar dari Berkas…** menaruh gambar ke dalam teks; Anda juga dapat menyeret gambar ke dalam teks atau menempelkannya. Klik kanan gambar dan pilih **Ukuran Gambar…** untuk mengubah seberapa besar gambar tampil.
- **Sisipkan > Tautan ke Dokumen…** (**Ctrl+L**) menautkan teks yang dipilih ke dokumen lain di proyek. Mengklik tautan membuka dokumen itu (di editor lain, editor yang sama, pemegang salinannya, atau panel Referensi Cepat; lihat **Berkas > Opsi > Perilaku**).
- **Sisipkan > Sisipkan Tautan Web…** menautkan ke halaman web.
- Tautan web dan surel (*http*, *https*, *mailto*) langsung terbuka. Tujuan tautan lainnya, di teks, peta pikiran, penanda, atau halaman web hasil impor (berkas, folder, alamat jaringan, jenis tautan program lain), ditampilkan utuh lebih dulu dan dibuka hanya ketika Anda mengklik **Buka**; tautan seperti itu dapat datang bersama proyek, halaman web, atau berkas Word milik orang lain. Untuk berkas atau folder di komputer Anda, pertanyaan menyebutkan jenisnya (misalnya "plain text document (.txt)"), nama lengkapnya, dan foldernya; nama yang panjang dibungkus, sehingga akhirannya dapat dibaca, dan berkas yang tidak ada dinyatakan hilang. Tautan ke program, skrip, pintasan, dan penginstal tidak pernah dibuka; bukalah sendiri dari pengelola berkas Anda bila Anda memercayainya.

### Tabel

**Sisipkan > Tabel…** menanyakan jumlah baris dan kolom serta desainnya, dan menaruh tabel pada kursor. Di dalam tabel, **Tab** menuju sel berikutnya (di sel terakhir menambah baris) dan **Shift+Tab** kembali. Untuk keluar dari tabel dengan papan ketik, tekan **Ctrl+Return** (ke bawah tabel) atau **Bawah** pada baris terakhir (ke bawahnya) atau **Atas** pada baris pertama (ke atasnya); bila di dalam sel masih ada satu baris lagi, Atas dan Bawah lebih dulu melewati baris-barisnya. Bila tidak ada paragraf di samping tabel, {{APP}} membuat satu paragraf kosong di sana (satu langkah untuk Urungkan). **Format > Tabel**, juga di menu klik kanan di dalam tabel, memiliki sisanya:

- **Sisipkan Baris di Atas / Bawah**, **Sisipkan Kolom di Kiri / Kanan**.
- **Hapus Baris** dan **Hapus Kolom** (milik sel yang dipilih, atau sel dengan kursor), **Hapus Tabel**.
- **Gabungkan Sel** menyatukan sel yang dipilih menjadi satu; **Pisahkan Sel** membagi sel gabungan lagi.
- **Warna Sel…** memberi sel yang dipilih latar belakang tersendiri; **Warna Sel dari Desain** menghilangkannya lagi.
- **Properti Tabel…** mengubah desain.

**Desain.** Lima desain sudah tersedia: **Kisi**, **Kisi dengan baris header**, **Baris belang**, **Garis di antara baris**, dan **Tanpa garis**. Pilih satu dan ubah sesukanya; desain itu kemudian menjadi **Milik saya**:

- **Garis**: lebar (0: tanpa garis), warna, dan **Hanya di antara baris** (tanpa garis di sisi).
- **Sel**: ruang di dalam sel, dan ruang di antara sel (0: sel yang bertetangga berbagi satu garis).
- **Baris header**: berapa baris di bagian atas yang merupakan judul, beserta warnanya. Pencetakan dan kompilasi mengulangnya di bagian atas setiap halaman yang dilanjutkan oleh tabel panjang. **Setiap baris lainnya** mendapat warna tersendiri bila Anda memilihnya.
- **Tempat di dalam teks**: lebar (bagian dari lebar teks, atau selebar isinya), lebar kolom (semua sama, atau bagian seperti "30, 70"), perataan, dan tempat berdirinya: **Pada barisnya sendiri**, atau **Kiri** atau **Kanan** dengan teks mengalir di sampingnya.

Baris dan kolom yang Anda tambahkan kemudian mempertahankan desain (belangnya berlanjut). Teks di sel tabel tidak rata kiri-kanan, juga bila teks isi demikian (sel sempit akan merentangkan katanya); perataan yang Anda berikan pada paragraf di dalam sel tetap. Tabel disimpan di kedua format penyimpanan dan di format Scrivener; tabel di berkas RTF dari program lain (misalnya berkas Word yang disimpan sebagai RTF) juga menjadi tabel.

### Gambar di samping teks, keterangan, dan deskripsi

Klik kanan gambar dan pilih **Gambar…** (atau taruh kursor di sampingnya dan pilih **Format > Gambar…**):

- **Lebar di layar**: tinggi mengikuti; **Ukuran Asli** kembali ke ukuran gambar sendiri.
- **Posisi**: **Di dalam baris, seperti huruf** (seperti sebelumnya), **Kiri** atau **Kanan** dengan teks mengalir di sekitarnya, atau **Di tengah pada barisnya sendiri**.
- **Keterangan**: ditampilkan di bawah gambar dan dikompilasi bersamanya. Gambar di dalam baris tidak memiliki keterangan; berikan posisi lebih dulu.
- **Deskripsi**: apa yang ditampilkan gambar, dalam satu atau dua kalimat. Tidak ditampilkan di teks. E-book, berkas Word, dan Markdown menyimpannya bersama gambar (sebagai "teks alternatif"-nya), dan pembaca layar membacakannya kepada orang yang tidak dapat melihat gambar.

Gambar beserta keterangannya bergerak sebagai satu kesatuan; ketik keterangan di bawah gambar seperti teks lainnya (gayanya **Caption**).

### Cari dan ganti

- **Ctrl+F** (**Edit > Cari > Cari…**) membuka bilah pencarian di atas teks. Bilah itu menunjukkan berapa banyak yang cocok dan menyorotnya; **F3** dan **Shift+F3** menuju kecocokan berikutnya dan sebelumnya. **Cocokkan huruf besar/kecil** dan **Kata utuh** mempersempit pencarian.
- **Ctrl+H** menambahkan baris ganti: **Ganti** mengubah kecocokan yang dipilih dan menuju yang berikutnya, **Ganti Semua** mengubah setiap kecocokan di teks yang Anda lihat (satu langkah untuk **Urungkan**). Teks baru mengambil pemformatan teks yang digantikannya, dan komentar tetap melekat.
- **Ctrl+E** (**Pakai Pilihan untuk Pencarian**) memasukkan teks yang dipilih ke bilah pencarian.
- Pada teks gabungan, cari dan ganti bekerja di semua dokumen yang ditampilkan.
- Selama papan ketik berada di **pemegang salinan**, Ctrl+F mencari di dokumen pemegang salinan.
- **Ctrl+Shift+F** mencari di seluruh proyek, **Edit > Cari > Ganti di Proyek** mengganti di seluruh proyek, dan **Edit > Cari > Cari menurut Pemformatan** mencari sorotan, komentar, warna, dan gaya (semuanya di bagian 9).
- Di papan gabus dan di kerangka, Ctrl+F membuka **bilah filter** sebagai gantinya (bagian 5).

### Ejaan

Kata yang salah eja mendapat garis bawah bergelombang merah; klik kanan di mana saja pada kata bergaris bawah itu untuk melihat saran atau menambahkan kata ke kamus Anda (juga pada kata dengan apostrof, seperti "Kayel's" atau "D'Nersun": menu selalu menawarkan seluruh kata yang bergaris bawah). Pada kata yang salah eja, menu dibuat singkat agar terbuka tepat di mouse: saran, **Tambahkan "..." ke Kamus**, **Abaikan "..."**, dan **Perintah Lainnya** dengan semua yang dimuat menu di tempat lain (Potong, Salin, Tempel, Gaya, komentar, Penampilan ...). Nama program sendiri dan istilahnya ({{APP}}, outliner, subdocument, copyholder) selalu dianggap benar. Pilih bahasa di **Berkas > Opsi > Koreksi** (halaman Ejaan); kamus yang Anda pasang sebagai pengaya (satu per bahasa program, masing-masing dengan lisensinya) ada dalam daftar itu; di sana Anda juga memilih garis bawahnya (gelombang, titik, atau garis lurus), apakah kata dengan angka ("Route66") diperiksa, dan **Lihat Daftar Kata Pribadi...** menampilkan kata yang Anda tambahkan dan menghapus yang Anda tambahkan karena salah. Kamus berasal dari sistem Anda dan dari pengaya. Di Ubuntu dan Kubuntu, bahasa Inggris sudah terpasang; untuk bahasa lain, pasang paket *hunspell* yang sesuai di Discover (misalnya "hunspell-de-de" untuk bahasa Jerman).

### Penyisipan khusus

Menu **Sisipkan** juga menawarkan tanggal dan waktu saat ini, karakter khusus (tanda pisah em, tanda pisah en, elipsis, spasi tanpa jeda, tanda seksi, poin), **nomor otomatis**, dan **placeholder**, yang diganti oleh Kompilasi (bagian 13).

### Koreksi saat mengetik

**Berkas > Opsi > Koreksi** menetapkan apa yang dikoreksi saat Anda mengetik. Sebuah kata dikoreksi ketika Anda menyelesaikannya, yaitu ketika Anda mengetik spasi, Return, atau tanda baca setelahnya; **Ctrl+Z** langsung sesudahnya membatalkan koreksi itu saja.

- **Perbaiki huruf kapital kalimat**: kata pertama kalimat mendapat huruf kapital (tidak setelah singkatan seperti "Dr." atau "e.g."). **Kapitalkan "i"**: "i" dan "i'm" menjadi "I" dan "I'm". Keduanya awalnya mati.
- **Tanda kutip cerdas** dalam gaya yang Anda pilih (“Inggris”, „Jerman“, »Jerman«, «Prancis», ”Swedia”); apostrof di dalam kata selalu ’. Bila tanda kutip tunggal penutup bukan apostrof (Jerman ‚…‘, Prancis ‹…›), ' yang diketik tepat setelah sebuah kata menutup tanda kutip tunggal yang terbuka di paragraf itu; bila Anda kemudian melanjutkan mengetik huruf ("geht's"), tanda itu menjadi apostrof lagi. Dua tanda hubung menjadi tanda pisah em (—); tiga titik dapat menjadi satu karakter elipsis (awalnya mati). Dalam mode skenario, semuanya dapat dimatikan.
- **Tanda kutip yang sudah ada di proyek** (sejak 0.61.1): di bawah **Tanda kutip cerdas**, **Konversi ke Tanda Kutip Tipografis…** mengubah " dan ' lurus di proyek menjadi tanda gaya yang dipilih di samping **Pakai tanda kutip cerdas**, dan **Konversi ke Tanda Kutip Lurus…** mengubah setiap tanda kutip tipografis dan apostrof kembali menjadi " dan '. Jendela seperti Ganti di Proyek (bagian 9) terbuka: arah dan gayanya sekali lagi, di mana akan dikonversi (judul, teks, catatan, sinopsis, metadata kustom, komentar dan catatan kaki), cakupan (seluruh proyek atau dokumen yang dipilih di binder, Tempat Sampah hanya bila Anda mencentangnya), dan snapshot setiap teks sebelum berubah (awalnya dicentang). **Hitung** menunjukkan berapa banyak tanda yang akan berubah; **Konversi** bertanya sekali lagi dan melakukannya. Pemformatan tetap (tanda kutip tebal tetap tebal). Tanda membuka setelah tidak ada apa-apa, spasi, tanda kurung, tanda pisah, atau tanda pembuka, dan menutup selain itu; apostrof di dalam kata atau sebelum angka ('90s) menjadi ’. Tanda lurus dengan spasi di kedua sisinya tetap seperti adanya, dan apostrof di awal kata ('tis) menjadi tanda pembuka: periksalah hal-hal itu sesudahnya. Cari dan Ganti tidak dapat melakukan ini, karena satu tanda harus menjadi tanda pembuka atau penutup tergantung tetangganya.
- **Deteksi data**: alamat web yang diketik menjadi tautan; **[[Title]]** menjadi tautan ke dokumen dengan judul itu, dan bila tidak ada, dokumen kosong baru dengan nama itu dibuat di samping teks ("tautan dokumen" Scrivener).
- **Substitusi tambahan**: daftar apa yang Anda ketik dan menjadi apa, misalnya "(c)" dan ©, "->" dan →. **Edit Substitusi...** mengubah daftar; **Substitusi Standar** mengembalikan daftar bawaan program.
- **Pelengkapan otomatis**: dalam mode skenario, daftar nama tokoh dan judul adegan yang ditawarkan saat Anda mengetik.

### Zoom, pengguliran mesin ketik, dan lebar tetap

- **Ctrl+roda mouse** melakukan zoom pada area di bawah mouse, di mana pun itu: editor, binder, inspektor, kerangka, papan gabus (kartu dan teksnya bersama-sama), peta pikiran, mode menulis, dan jendela tersendiri (Referensi Cepat, alat menulis); papan ketik boleh berada di tempat lain. Setiap area mempertahankan zoomnya sampai Anda mengubahnya lagi.
- **Ctrl++ / Ctrl+-** atau kotak zoom di kaki memperbesar teks di layar tanpa mengubah ukuran font. **Tampilan > Atur Ulang Zoom** (**Ctrl+0**) kembali ke zoom teks bawaan.
- **Berkas > Opsi > Pengeditan** menetapkan zoom awal setiap area ("Zoom teks bawaan", untuk setiap area jendela dan setiap bagian yang memiliki zoom sendiri: sinopsis, catatan, daftar penanda, teks penanda, daftar dan teks snapshot, daftar dan lembar objek di inspektor dan di jendela Objek, peta pikiran baru) dan apakah editor dibuka kembali dengan zoom terakhirnya ("Muat ulang zoom terakhir yang dipilih editor utama").
- **Di mana Anda berada.** {{APP}} mengingat, bersama proyek, tempat kursor dan bagian teks di layar untuk setiap dokumen: ditampilkan lagi (dari binder, tautan, Kembali dan Maju, di pemegang salinan, panel Referensi Cepat, atau setelah membuka kembali proyek), dokumen terbuka di tempat Anda meninggalkannya. Folder di teks gabungan mengingat dokumennya yang memiliki kursor. Hasil pencarian atau tautan yang menunjuk ke tempat tertentu menuju ke sana sebagai gantinya. Gambar mempertahankan zoomnya dan berkas PDF zoom serta halamannya.
- **Pengguliran mesin ketik** (**Tampilan > Pengeditan Teks**) menjaga baris yang sedang Anda ketik tetap di tengah layar.
- **Editor lebar tetap** menampilkan teks sebagai kolom di tengah dengan lebar yang nyaman (500 px pada 100 % kecuali Anda mengubahnya), selebar apa pun jendelanya. Matikan, atau ubah lebarnya, di **Berkas > Opsi > Penampilan > Editor Utama** ("Pakai editor lebar tetap", "Lebar kolom teks"), atau dengan klik kanan pada kertas (**Penampilan > Lebar Kolom Teks**). Kertas kolom itu #d1d495 dan area di sampingnya #999999 secara standar ("Kertas" dan "Di samping kolom teks" di antara warna Editor Utama); keduanya tetap sama ketika Anda memilih tema lain, sampai Anda mengubah atau mengatur ulangnya.
- **Batas di samping teks.** Di kiri dan kanan teks, kertas memiliki batas dengan warnanya sendiri, agar teks tidak menyentuh tepi kertas: 10 px pada 100 % untuk proyek baru, dan membesar mengikuti zoom. Batas itu berada di luar kolom teks, sehingga baris terpotong di tempat yang sama. Ubah di **Berkas > Opsi > Penampilan > Editor Utama** ("Batas di samping teks", 0 hingga 100 px) atau dengan klik kanan pada kertas (**Penampilan > Batas di Samping Teks**). Proyek yang dibuat sebelum 0.40.2 mempertahankan tampilan yang dimilikinya, tanpa batas, sampai Anda memberinya batas. Klik pada batas menaruh kursor di awal atau akhir baris di sampingnya.
- **Mengklik dan mengklik kanan** tidak pernah menggeser tampilan: kursor menuju ke tempat Anda mengklik, dan teks tetap di tempatnya di layar (sebelum 0.40.2, klik atau klik kanan di dekat tepi atas atau bawah, atau klik kanan di samping teks, dapat membuat teks melompat).

### Tampilan halaman

**Tampilan > Pengeditan Teks > Tampilan Halaman** (juga di menu tombol **Tampilan** pada bilah alat) menampilkan teks Anda di lembar kertas, bukan satu kolom tanpa akhir. Kaki kemudian menampilkan halaman kursor dan jumlah halaman, misalnya "Page 3 of 12". Pilih perintah itu lagi untuk kembali ke kolom. Warna meja di sekitar halaman ada di **Berkas > Opsi > Penampilan > Editor Utama** (atau klik kanan > **Penampilan** selagi halaman ditampilkan).

Halaman hanyalah cara memandang teks Anda, tidak lebih. Halaman tidak menunjukkan seperti apa naskah yang dikompilasi atau dicetak: Anda menulis dalam potongan-potongan, dan judul, font, serta ukuran halaman ditentukan saat Anda mengompilasi (bagian 13). Tidak ada apa pun tentang halaman yang disimpan di dokumen Anda.

- **Tampilan > Pengeditan Teks > Ukuran dan Margin Halaman…** menetapkan kertas (A4, A5, US Letter, dan lainnya, atau ukuran Anda sendiri), potret atau lanskap, dan keempat margin. Pengaturan itu milik proyek; centang **Pakai halaman ini untuk proyek baru juga** untuk menjadikannya standar Anda.
- Zoom memperbesar lembar, margin, dan huruf bersama-sama. Lembar yang lebih lebar daripada editor dapat digulir ke samping. Seperti di tampilan kolom, indentasi dan perhentian tab mempertahankan panjang sebenarnya, sehingga pemotongan baris dapat sedikit berbeda antar tingkat zoom.
- Di teks gabungan, setiap dokumen dimulai pada lembarnya sendiri, dan lembar terakhirnya berakhir tepat di bawah teks, sehingga dokumen pendek tidak memenuhi layar dengan kertas kosong.
- Tampilan halaman berlaku untuk semua yang menampilkan teks Anda di jendela utama dan di jendela referensi cepat: kedua editor dan pemegang salinan. Catatan mempertahankan tampilan polosnya, dan mode menulis memiliki kertasnya sendiri.
- Warna di sekitar lembar adalah **Di sekitar halaman (tampilan halaman)** di **Berkas > Opsi > Penampilan > Editor Utama**.

### Penulisan skenario

Skenario film, naskah drama panggung, dan skrip komik ditulis dalam **mode skenario**. Skenario terdiri dari *elemen*: judul adegan, aksi, tokoh, parentetikal, dialog, transisi, dan seterusnya. Setiap elemen memiliki tempatnya di halaman, dan dalam mode skenario tombol **Tab** dan **Enter** membawa Anda dari satu elemen ke elemen berikutnya, sehingga Anda tidak pernah menetapkan indentasi secara manual.

**Menyalakannya.** Klik di dalam dokumen dan pilih **Format > Penulisan Skenario > Mode Skenario** (**Ctrl+8**). Entri menu menyebutkan format skenario proyek, misalnya *Script Mode - Screenplay*. Mode skenario milik dokumen: skenario dan teks biasa hidup berdampingan dalam satu proyek, dan skenario memiliki **ikon kuning** di binder. Pilih perintah itu lagi untuk mematikannya; teks mempertahankan tampilannya. Dokumen baru yang dibuat di antara skenario (di folder yang semua teksnya adalah skenario) juga menjadi skenario.

**Kaki.** Dalam skenario, kaki editor menampilkan, sebagai pengganti jumlah kata, apa yang akan dilakukan **Tab** dan **Enter** pada kursor, dan di kanan menu pop-up dengan elemen paragraf saat ini. Pilih elemen lain dari menu itu dan paragraf diformat ulang. Teks yang bukan elemen skenario ditampilkan sebagai **General Text**.

**Tab dan Enter.**

- **Enter** di akhir paragraf memulai elemen yang biasanya mengikuti: aksi setelah judul adegan, dialog setelah nama tokoh, dan seterusnya.
- **Enter** pada baris kosong membuka daftar elemen pada kursor; tekan huruf yang bergaris bawah di daftar (S untuk judul adegan, C untuk tokoh ...). **Format > Penulisan Skenario > Tampilkan Daftar Elemen** (**Ctrl+\\**) membuka daftar yang sama kapan saja.
- **Tab** pada baris kosong mengubah baris menjadi elemen lain; tekan lagi untuk yang berikutnya, **Shift+Tab** untuk kembali.
- **Tab** di akhir paragraf melakukan apa yang cocok untuk elemen itu: setelah nama tokoh atau baris dialog, tombol ini memulai parentetikal, dan di judul adegan, tombol ini mengetik " - ", siap untuk waktu dalam sehari.
- **Enter** di tengah paragraf memisahkannya; kedua bagian mempertahankan elemennya. **Shift+Enter** adalah pemisah baris di dalam paragraf.
- Elemen yang ditulis dengan huruf kapital (judul adegan, nama tokoh, transisi) mengubah apa yang Anda ketik menjadi huruf kapital. Tanda kurung dan awalan seperti "SFX: " diketikkan untuk Anda.

**Pelengkapan otomatis.** Saat Anda mengetik, daftar kecil menawarkan apa yang cocok untuk elemen itu: INT. dan EXT. di awal judul adegan, waktu dalam sehari setelah " - "-nya, tempat dan nama tokoh yang sudah dipakai di dokumen, dan transisi baku seperti CUT TO:. **Enter** atau **Tab** mengambil entri yang ditandai, tombol panah memilih yang lain, **Esc** menutup daftar.

**Mengubah elemen.** **Format > Penulisan Skenario > Ubah Elemen Menjadi** mengubah paragraf dengan kursor, atau semua paragraf yang dipilih. Memilih elemen di sana pada dokumen biasa menjadikan dokumen itu skenario. Setiap elemen adalah perintah, sehingga Anda dapat memberi pintasan pada favorit Anda di **Berkas > Opsi > Papan Ketik**.

**Format skenario.** Sebuah proyek memakai satu format skenario untuk semua skenarionya. **Format > Penulisan Skenario > Format Skenario** mencantumkan *Screenplay*, *Stage Play*, dan *Comic Script*, format dari pengaya, dan format Anda sendiri. Bila Anda memilih format lain, skenario proyek mendapat tata letak yang baru: setiap elemen menjadi elemen sejenis di format baru (aksi menjadi arahan panggung, misalnya). Elemen tanpa padanan menjadi teks umum yang mempertahankan tampilannya; pertanyaan sebelum perubahan menyebutkan elemen-elemen ini.

**Pengaturan Skenario.** **Format > Penulisan Skenario > Pengaturan Skenario…** mengubah format proyek atau membuat yang baru:

- nama format, fontnya (secara bawaan font mesin ketik sistem Anda: Courier Prime, Courier New, atau font serupa) dan ukurannya, dan apakah panjang dalam inci atau sentimeter;
- daftar elemen: tambah, hapus, dan ubah urutannya (urutan itu adalah urutan daftar elemen dan **Tab** pada baris kosong);
- untuk setiap elemen: nama, jenis (apa itu bagi program lain: judul adegan, aksi, tokoh ...), dan hurufnya di daftar; tebal, miring, bergaris bawah, kapital, dan warna; perataan, indentasi dari kedua margin, baris kosong sebelumnya, dan jarak baris; apa yang dilakukan **Enter** dan **Tab**; teks yang diketik sebelum dan sesudah paragraf; dan daftar untuk pelengkapan otomatis;
- pratinjau semua elemen.

Elemen yang diganti namanya mempertahankan paragrafnya di setiap skenario proyek. Tombol **Kelola** memulai dari format lain, memuat atau menyimpan berkas format (*.qbscript*), dan menyimpan format untuk dipakai di proyek lain.

**Baik untuk diketahui.**

- Tombol dan daftar bekerja sama di teks gabungan, di editor kedua, di pemegang salinan, di panel Referensi Cepat, dan di mode menulis. Hanya editor di jendela utama yang memiliki kaki dengan petunjuk; di tempat lain **Ctrl+\\** menampilkan daftar elemen.
- Tampilan halaman (di atas) menampilkan skenario di lembar kertas; dengan font mesin ketik yang lazim pada 12 poin, satu halaman skenario film kira-kira satu menit film, tetapi halaman di layar bukan halaman berkas yang dikompilasi.
- Font dan indentasi elemen milik format skenario, bukan milik dokumen. Berkas skenario Anda hanya memuat nama elemen (bagian 18).
- Memasukkan dan mengeluarkan skenario, dalam format program penulisan skenario lain: bagian 14. Mengompilasi seluruh skenario: bagian 13.
- Skenario memerlukan versi 0.10 atau lebih baru. Proyek yang memuat skenario menyatakannya di berkas proyeknya, dan sejak versi ini {{APP}} bertanya sebelum membuka proyek yang memerlukan versi lebih baru daripada yang sedang berjalan. Versi 0.9 dan yang lebih lama tidak bertanya: mereka menampilkan skenario sebagai teks biasa dan kehilangan elemen setiap skenario yang Anda lihat di sana.

---

## 5. Tiga cara memandang proyek Anda

Pilih tampilan dengan ketiga tombol bilah alat atau dengan **Ctrl+1**, **Ctrl+2**, dan **Ctrl+3**.

### Teks Gabungan (Ctrl+1): banyak dokumen sebagai satu teks

Pilih beberapa dokumen di binder (Ctrl+klik atau Shift+klik) atau sebuah folder, lalu tekan **Ctrl+1**. Semua teks muncul satu demi satu, dipisahkan oleh garis putus-putus tipis, dan Anda dapat menulis di semuanya seolah-olah satu dokumen. Perubahan masuk ke dokumen yang tepat secara otomatis. Tampilkan judul dokumen di antaranya dengan **Tampilan > Pengeditan Teks > Tampilkan Judul di Teks Gabungan**.

### Papan Gabus (Ctrl+2): kartu indeks

Setiap dokumen adalah kartu indeks yang menampilkan judul dan sinopsisnya; dokumen tanpa sinopsis menampilkan baris-baris pertama teksnya sebagai gantinya (dipudarkan, dicetak miring). Kartu gambar menampilkan gambar itu, demikian pula kartu dokumen yang sinopsisnya berupa gambar (bagian 7). Seret kartu untuk mengurutkan ulang dokumen. Klik dua kali kartu untuk mengedit judul atau sinopsisnya. Tekan **Enter** untuk membuka kartu yang dipilih sebagai teks.

Di kaki papan gabus:

- **Kisi** atau **bebas**: dalam mode bebas Anda dapat menaruh kartu di mana saja, seperti di papan gabus sungguhan. **Terapkan Urutan** kemudian mengurutkan ulang dokumen di binder agar sesuai dengan kartu dari kiri ke kanan, atas ke bawah. Penataan yang Anda pilih untuk sebuah folder diingat bersama folder itu (sejak 0.90.0): saat berikutnya Anda menampilkannya, papan gabusnya ditata dengan cara yang sama. Folder yang tidak pernah Anda pilihkan penataannya mempertahankan penataan yang ditampilkan editor.
- **Benang label**: menata kartu dalam baris menurut label.
- **Ukuran kartu**, dengan penggeser, tombol panah, atau roda mouse di atasnya; langsung disimpan (**Ctrl+roda mouse** di atas papan gabus memperbesar kartu dan teksnya bersama-sama, seperti area lain, lihat bagian 4).

Klik dua kali pada tempat kosong di papan gabus membuat kartu baru (**Berkas > Opsi > Perilaku > Klik Dua Kali**). Ketika Anda mengedit sinopsis pada kartu, **Return** mengakhiri pengeditan dan **Shift+Return** membuat baris baru (**Berkas > Opsi > Perilaku > Tombol Return**, tempat Return juga dapat ditetapkan untuk membuat kartu atau baris kerangka baru setelah yang dipilih).

Stempel status, mode bebas, dan penataan menurut label juga ada di **Tampilan > Opsi Papan Gabus**; rona label untuk kartu dan binder di **Tampilan > Gunakan Warna Label di**. Kolom kerangka juga dicantumkan di **Tampilan > Opsi Kerangka**.

### Kerangka (Ctrl+3): sebuah tabel

Menampilkan dokumen sebagai tabel dengan judul, sinopsis, label, status, jumlah kata, target, tanggal, dan metadata kustom Anda. Klik tombol kolom di kaki untuk memilih kolom. Klik header kolom untuk mengurutkan (hanya untuk dilihat; urutan binder tidak berubah). Seret baris untuk mengurutkan ulang dokumen.

### Menyaring papan gabus dan kerangka

Selama papan gabus atau kerangka memegang papan ketik, **Ctrl+F** (**Edit > Cari > Cari…**) membuka **bilah filter** di bagian atas editor. Saat Anda mengetik, hanya kartu atau baris yang cocok yang tersisa; bilah menyebutkan berapa banyak ("2 of 9"). Di kerangka, grup dokumen yang cocok tetap ada agar Anda melihat tempatnya.

- Klik **segitiga** di kiri kolom untuk opsi: di mana mencari (**Judul**, **Sinopsis**, **Kata Kunci**, **Teks**), sebuah **label**, sebuah **status**, sebuah **kata kunci**, apakah dokumen **disertakan dalam kompilasi**, dan **Tampilkan yang tidak cocok**. Beberapa pilihan bersama-sama mempersempit daftar lebih jauh.
- Filter tetap ada selagi Anda membuka folder lain, sehingga Anda dapat menelusuri proyek sambil melihat hanya, misalnya, adegan seorang tokoh.
- **Selesai** menghapus filter dan menampilkan semuanya lagi.

Menyeret kartu tetap mengurutkan ulang dokumen: kartu yang dijatuhkan di antara dua kartu yang Anda lihat mendarat di samping keduanya di binder. **Terapkan Urutan** pada papan gabus bebas dimatikan selama filter menyembunyikan kartu.

---

## 6. Dua editor, penanda, dan referensi cepat

### Editor terpisah

Klik tombol pisah di sisi kanan header editor untuk menampilkan editor kedua. Alt+klik memisahkan ke arah lain. **Tampilan > Pisahkan Editor** menawarkan pilihan yang sama. **F6** melompat di antara editor. Mengklik di binder menampilkan dokumen di editor yang terakhir Anda pakai; **Navigasi > Pilihan Binder Memengaruhi** mengubahnya menjadi editor kiri atau atas, editor kanan atau bawah, atau editor lain, mana pun yang sedang Anda tulis.

Di papan gabus atau kerangka, tombol kaki **Pilihan memengaruhi editor lain** (juga di **Tampilan > Opsi Papan Gabus** dan **Opsi Kerangka**) membuat editor lain menampilkan kartu yang Anda klik. Ini cara cepat untuk menjelajahi adegan sambil mempertahankan ikhtisar bab.

**Navigasi > Editor > Kunci di Tempat** mempertahankan editor pada dokumennya, bahkan ketika Anda mengklik di tempat lain di binder.

### Pemegang salinan

Setiap editor memiliki **pemegang salinan**: tampilan kedua yang polos di dalam editor yang menyimpan satu dokumen lain sebagai rujukan, seperti catatan atau lembar tokoh. Dengan dua editor dan pemegang salinannya, Anda dapat melihat empat dokumen sekaligus.

- **Untuk membukanya**, tahan **Alt** dan seret dokumen dari binder, papan gabus, atau kerangka ke bilah header editor. Tanpa Alt, penjatuhan memuat dokumen ke editor itu sendiri. **Navigasi > Buka > Di Pemegang Salinan** dan menu klik kanan dokumen melakukan hal yang sama untuk dokumen yang dipilih.
- Pemegang salinan memiliki bilah header sempit dengan judul dan tanpa kaki. Ia menampilkan teks satu dokumen (atau gambar atau PDF); ia tidak memiliki papan gabus, kerangka, atau teks gabungan. Anda dapat menulis dan memformat di dalamnya seperti di editor mana pun.
- **Untuk memindahkannya**, klik kanan bilah headernya dan pilih kiri, kanan, atas, atau bawah. Pertama kali, ia terbuka di samping editor lebar dan di bawah editor sempit; setelah itu, di tempat Anda terakhir menaruhnya. Seret garis di antara editor dan pemegang salinan untuk mengubah ukurannya.
- **Untuk menampilkan dokumen lain**, jatuhkan di bilah header pemegang salinan, atau Alt+seret lagi ke bilah header editor.
- **Untuk menutupnya**, klik **X** di sisi kanan bilah headernya, atau pilih **Navigasi > Editor > Tutup Pemegang Salinan**. Tombol di sebelah kiri X menutupnya dan membuka dokumen sebagai jendela referensi cepat sebagai gantinya.

**Bernavigasi dengan pemegang salinan.** Selama editor menampilkan papan gabus atau kerangka dan pemegang salinannya terbuka, tombol **Pilihan memengaruhi** di kaki memiliki tiga posisi. Klik pertama membuat pilihan Anda terbuka di editor lain, seperti biasa. Klik kedua memutar panah sehingga menunjuk ke dalam kotak: sekarang kartu atau baris yang Anda klik terbuka di pemegang salinan. Klik ketiga mematikannya. Penataan yang berguna: naskah di editor kiri, kerangka riset Anda di editor kanan dengan pemegang salinannya di bawah, dan **Navigasi > Pilihan Binder Memengaruhi > Editor Kiri atau Atas**. Binder kemudian mengendalikan naskah, dan kerangka mengendalikan pemegang salinan.

Pemegang salinan, tempatnya, dan dokumennya diingat bersama proyek.

### Header editor

Header di atas setiap editor menampilkan judul dokumen dan memuat tombol **kembali** dan **maju** (**Ctrl+[** dan **Ctrl+]**), menu bagian-bagian dokumen, dan tombol untuk dokumen sebelumnya dan berikutnya (**Alt+Shift+Atas / Bawah**).

- **Riwayat**: setiap editor mengingat dokumen yang pernah ditampilkannya. Tahan tombol kembali atau maju (atau klik kanan) untuk melihat daftar dan pilih dokumen untuk langsung menuju ke sana; **Navigasi > Mundur ke** dan **Maju ke** menampilkan daftar yang sama. Riwayat disimpan bersama proyek.
- **Ikon** di samping judul mewakili dokumen. Klik untuk menu (Kunci di Tempat, Tunjukkan di Binder ...). Seret seperti dokumen di binder: ke bilah header editor lain, ke binder, ke tab koleksi, ke tombol Referensi Cepat atau Penanda pada bilah alat, atau ke teks untuk membuat tautan.
- **Dokumen > Grup** (**Ctrl+Shift+J**, juga di menu konteks binder) memindahkan item yang dipilih ke folder baru di tempat yang pertama; judulnya siap diketik. Dengan papan ketik di binder, **Edit > Urungkan** membatalkan seluruh langkah itu.
- **Dokumen > Konversi > Konversi Lembar menjadi Objek** (juga di menu konteks binder) membuat Objek (bagian tentang Objek) dari setiap dokumen lembar yang dipilih, misalnya lembar tokoh atau tempat: judul menjadi nama, teks masuk ke **Lainnya**, gambar di teks menjadi gambar. Nama folder (Characters, Places) menentukan jenisnya; untuk folder lain Anda ditanya. Item binder tetap ada: memilihnya menampilkan Objek di inspektor, dan Return membukanya di jendela Objek. Dokumen mempertahankan teksnya (proyek Scrivener melihat dokumen biasa); tautannya disimpan di data Objek milik proyek sendiri. **Edit > Urungkan** dengan papan ketik di binder membatalkannya.
- **Navigasi > Buka ke** mencantumkan seluruh binder sebagai menu dan memuat dokumen yang dipilih ke editor aktif. **Navigasi > Grup Pembungkus** menampilkan folder atau grup tempat dokumen berada, dengan dokumen terpilih di sana.
- **Klik dua kali pada pembatas** di antara dua editor membuat keduanya berukuran sama.

### Seret dan jatuhkan sekilas

| Seret ... | ke ... | dan ... |
|---|---|---|
| dokumen (binder, koleksi, papan gabus, kerangka, ikon header, penanda) | bilah header editor | ditampilkan di editor itu; dengan **Alt**: di pemegang salinannya |
| | teks di editor | tautan ke dokumen; pada teks yang dipilih, teks itu menjadi tautan |
| | teks, dengan **Alt** ditahan | teks dokumen disisipkan |
| gambar dari binder | teks di editor | gambar disisipkan |
| dokumen | tombol **Referensi Cepat** pada bilah alat | setiap dokumen terbuka di panelnya sendiri |
| | tombol **Penanda** pada bilah alat | ditambahkan ke penanda proyek |
| | tab koleksi, daftar penanda | ditambahkan |
| | bilah header panel Referensi Cepat | ditampilkan di panel itu |
| | pengelola berkas atau desktop | disimpan di sana sebagai berkas (lihat "Mengekspor dokumen") |
| teks yang dipilih | binder | dokumen baru dengan teks itu; teks tetap di tempatnya |
| berkas dari pengelola berkas | binder atau papan gabus | diimpor |
| tautan web dari peramban | folder di binder, papan gabus, kerangka | halaman diimpor (pengaya Impor Halaman Web, bagian 14) |
| berkas gambar, atau gambar dari binder | kartu indeks di inspektor (mode gambar) | menjadi gambar kartu |
| kata kunci dari **Proyek > Kata Kunci Proyek** | dokumen di binder, koleksi, papan gabus, kerangka; daftar kata kunci inspektor | ditetapkan; dengan **Alt**: bersama grup tempat kata kunci itu berada |

Beberapa desktop memakai Alt+seret untuk memindahkan jendela. Jika Alt+seret tidak sampai ke program, ubah tombol pengubah untuk memindahkan jendela di pengaturan desktop Anda (KDE: System Settings, perilaku jendela; tempat pastinya berbeda antar versi), atau gunakan perintah menu sebagai gantinya (**Navigasi > Buka > Di Pemegang Salinan**, **Proyek > Kata Kunci Proyek > Tetapkan ke Dokumen Terpilih**).

### Panel Referensi Cepat

Panel Referensi Cepat adalah dokumen di jendelanya sendiri, misalnya untuk menjaga lembar tokoh tetap terlihat selagi Anda menulis. Teks di panel dapat diedit dan diformat seperti di editor (pintasan menu Format dan Sisipkan bekerja di sana).

**Untuk membuka panel:**

- **Navigasi > Buka > Sebagai Referensi Cepat**, atau tombol Referensi Cepat pada bilah alat, untuk dokumen yang dipilih.
- Tekan **Spasi** di papan gabus atau di kerangka, atau di binder dan di daftar koleksi atau hasil pencarian ketika **Tampilan > Tambah Cepat Tombol di Binder** menyala (lihat bagian 2, tombol-tombol binder). Tanpa opsi itu, Spasi di binder digunakan untuk mengetik nama yang akan dilompati.
- Seret dokumen ke **tombol Referensi Cepat** pada bilah alat: masing-masing terbuka di panelnya sendiri, dan editor mempertahankan dokumennya.
- **Navigasi > Buka Referensi Cepat** menampilkan seluruh binder sebagai menu, sehingga Anda dapat membuka dokumen tanpa memilihnya lebih dulu.

**Di dalam panel:**

- **Judul** di bilah header dapat diedit (mengganti nama dokumen). **Ikon** di sampingnya membuka menu (Tunjukkan di Binder, Buka di Editor, tambahkan ke atau hapus dari penanda proyek) dan dapat diseret seperti dokumennya sendiri. Jatuhkan dokumen di bilah header untuk menampilkannya di panel ini.
- Tombol di **kanan atas** menampilkan **sinopsis**, **catatan**, **kata kunci**, atau **metadata** dokumen di bawah teksnya. Apa yang Anda ubah di sana mengubah dokumen.
- **Bilah kaki** menampilkan jumlah kata dan memungkinkan Anda menetapkan **label** dan **status**.
- **Tombol penanda** di sebelah kiri bilah kaki membuka **penanda proyek** di samping dokumen. Klik pada dokumen yang diberi penanda menampilkannya di panel, sehingga satu panel cukup untuk berpindah di antara catatan Anda. Seret penanda ke urutan lain, jatuhkan dokumen ke dalamnya, dan hapus penanda dengan tombol ➖ (dokumen tetap di binder). Berkas dan tautan web terbuka dengan klik dua kali.
- Tombol ➕ di bawah penanda membuat **dokumen baru** dan memberinya penanda. Pertama kali, Anda ditanya folder (misalnya folder "Notes" di Research); centang **Jangan tanya lagi** untuk memakai folder itu mulai sekarang. Ubah di **Proyek > Pengaturan Proyek > Folder Khusus**.
- Tombol **Ambang** menjaga panel di atas jendela utama; **Jendela > Ambangkan Panel Referensi Cepat** melakukannya untuk semuanya.

Menu Jendela mencantumkan panel yang terbuka dan menutup semuanya sekaligus. Panel yang terbuka saat Anda menutup proyek kembali ketika Anda membukanya lagi, dengan tempat, ukuran, panel bawah, dan bilah sampingnya (matikan di **Berkas > Opsi > Perilaku**).

### Audio dan video

Berkas audio dan video di binder (misalnya wawancara di folder Research) diputar di dalam {{APP}} ketika pengaya **Pemutaran Media** terpasang (bagian 17; datang bersama pembaruan 0.12). Tanpa pengaya itu, berkas seperti itu menampilkan **Buka dengan Aplikasi Bawaan**, seperti sebelumnya.

**Memutar.** Klik berkas di binder. Video menampilkan gambarnya; berkas audio menampilkan judul, artis, dan panjangnya. **Bilah kaki** editor memiliki kontrol putar:

- tombol **jeda dan putar ulang** di kiri (lihat di bawah),
- **buka ke awal**, **lompat mundur**, **putar / jeda**, **lompat maju**,
- **posisi**: seret penggeser, atau klik di samping pegangannya,
- waktu, **kecepatan** (0,5× hingga 2×; ucapan yang lebih lambat lebih mudah diketik; suara menjadi lebih rendah atau lebih tinggi mengikuti kecepatan, seperti pita kaset), dan **volume** (klik: bisukan; panah: penggeser).

Klik pada gambar video memutar atau menjedanya; klik kanan juga menawarkan **Buka dengan Aplikasi Bawaan**. Dengan papan ketik di pemutar: **Spasi** memutar atau menjeda, **tombol panah kiri** dan **kanan** melompat, **Home** menuju awal.

**Mentranskripsi.** Pisahkan editor, tampilkan berkas audio di satu editor dan transkrip Anda di editor lain, lalu terus mengetik: perintah **Navigasi > Media** bekerja di mana pun papan ketik berada, juga di panel Referensi Cepat dan mode menulis.

| Perintah | Pintasan |
|---------|----------|
| Putar / Jeda | Ctrl+Return |
| Lompat Mundur / Lompat Maju (5 detik) | Ctrl+Alt+, / Ctrl+Alt+. |
| Sisipkan Stempel Waktu Media (juga **Sisipkan > Sisipkan Stempel Waktu Media**: perintah yang sama, ditampilkan di kedua menu) | Ctrl+Shift+Return |
| Berhenti, Buka ke Awal, Jeda dan Putar Ulang, Lebih Cepat, Lebih Lambat, Kecepatan Normal, Naikkan Volume, Turunkan Volume, Bisukan, Buka ke Stempel Waktu pada Kursor | tanpa pintasan (berikan di **Berkas > Opsi > Papan Ketik**) |

Perintah bekerja pada berkas yang Anda buka atau pakai terakhir (di editor mana pun, pemegang salinan, atau panel Referensi Cepat).

**Jeda dan putar ulang.** Bila dinyalakan (tombol tampak tertekan), setiap jeda mundur sedikit, 2 detik kecuali Anda menetapkan nilai lain di **Berkas > Opsi > Perilaku > Pemutaran**. Ketika Anda menekan Ctrl+Return untuk mengetik apa yang Anda dengar, Anda mendengar kata-kata terakhir lagi saat melanjutkan.

**Stempel waktu.** **Sisipkan Stempel Waktu Media** mengetik waktu pemutar pada kursor, seperti `[03:07]` (dengan jam mulai dari satu jam: `[1:02:03]`). **Buka ke Stempel Waktu pada Kursor** melakukan sebaliknya: mengambil stempel waktu di baris kursor (yang terdekat sebelum atau di sekitarnya) dan memindahkan pemutar ke sana. Stempel waktu adalah teks biasa; juga dapat ditulis secara manual (`3:07`, `[03:07]`, `1:02:03.5`).

**Di pemegang salinan atau panel Referensi Cepat** pemutar memiliki kontrol putarnya di bagian bawah. Berkas audio di pemegang salinan editor transkrip adalah penataan yang baik untuk satu editor saja.

**Di mana terakhir berhenti.** Ketika Anda membuka berkas audio atau video lagi, berkas dilanjutkan dari tempat Anda menjedanya (matikan di **Berkas > Opsi > Perilaku > Pemutaran**). Menampilkan dokumen lain di editor menghentikan berkas.

**Apa yang diputar.** Pengaya *Pemutaran Media* (2.0) memutar format yang bebas dipakai: audio MP3, Ogg Vorbis, Opus, FLAC, ALAC, WAV, AIFF, dan WavPack; video WebM, MKV, dan Ogg dengan gambar VP8, VP9, atau Theora (juga MJPEG dan format bebas lainnya). Pengaya membawa semua yang diperlukannya.

**Format lainnya.** H.264, H.265 (HEVC), AAC (M4A), AC-3, E-AC-3, DTS, video MPEG-1/2/4, VC-1 dan WMV, WMA, dan AMR dilisensikan berdasarkan paten di beberapa negara; format-format itu datang dengan pengaya terpisah **Format Video dan Audio Tambahan**. Cara paling sederhana untuk mendapatkan keduanya adalah satu paket **media-all-formats.qbpkg** di halaman Releases: dipasang seperti setiap pengaya (bagian 17), paket itu memasang *Pemutaran Media* dan *Format Video dan Audio Tambahan* sekaligus. Keduanya kemudian tampil di **Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya**, tempat format tambahan dapat dimatikan atau dihapus tersendiri. Apakah lisensi paten diperlukan untuk memakai format-format ini bergantung pada negara. Selama terpasang dan menyala, *Pemutaran Media* memakainya, dan berkas MP4/M4V/MOV, AVI, MPEG, WMV, FLV, dan 3GP juga diputar (bukan WMA Pro dan WMV 9). Tanpanya, berkas seperti itu menampilkan pesan di bawah pemutar yang menyebutkan formatnya dan pengaya ini; semua yang lain bekerja seperti sebelumnya. Video AV1 tidak diputar dengan dekode bawaan.

Sejak 0.80.0 pemutar selalu mendekode dengan FFmpeg miliknya sendiri: Qt yang dipakai {{APP}} sejak itu (build Qt 6.12 buatan The Qt Company sendiri) tidak memiliki cara untuk memutar melalui **GStreamer** milik sistem, sehingga **Berkas > Opsi > Perilaku > Pemutaran > Dekode** menampilkan pilihan itu sebagai tidak tersedia.

### Penanda

**Penanda dokumen** (inspektor, tab penanda) menautkan sebuah dokumen ke dokumen lain, berkas di disk Anda, atau halaman web. **Penanda proyek** tersedia dari setiap dokumen, dari tombol penanda di bilah alat, dan dari bilah samping panel Referensi Cepat. Tab penanda di inspektor menampilkan kedua daftar, satu di atas yang lain: **Penanda Dokumen** dan **Penanda Proyek**, masing-masing dengan tombol ➕ dan ➖ sendiri. Klik pada penanda menampilkan dokumen di bawah daftar; dokumen yang sudah ditampilkan editor muncul di sana sebagai salinan baca-saja (menulislah di editor), sehingga editor mempertahankan lebar barisnya. Klik dua kali membuka penanda. Seret dokumen, berkas, atau tautan web ke daftar penanda, atau gunakan tombol ➕; tautan yang diseret dari peramban web mempertahankan judul halamannya. Seret penanda ke atas atau ke bawah untuk mengubah urutan. Pembatas di antara kedua daftar dan teks di bawahnya dapat digeser; ukuran yang Anda berikan disimpan bersama proyek dan kembali setelah dimulai ulang atau pembaruan program, juga ketika saat itu tidak ada penanda yang dipilih. Dokumen yang dijatuhkan pada **tombol Penanda** di bilah alat menjadi penanda proyek; demikian pula dokumen yang dipilih dengan **Dokumen > Tambahkan ke Penanda Proyek**; inspektor kemudian menampilkan tab penandanya dengan entri baru terpilih.

---

## 7. Inspektor

Tombol-tombol di bagian atas inspektor beralih antara:

- **Sinopsis dan catatan**: sinopsis adalah ringkasan singkat yang muncul pada kartu indeks. Anda dapat memakai gambar sebagai ganti teks: klik tombol gambar pada kartu dan jatuhkan berkas gambar atau gambar dari binder pada area abu-abu (atau klik dua kali area itu untuk memilih berkas; klik kanan untuk menghapus gambar). Di bawahnya terdapat **catatan dokumen** (atau, dengan menu kecil, **catatan proyek**, yang dipakai bersama oleh seluruh proyek). **Navigasi > Inspeksi > Buka ke Sinopsis** (**Ctrl+Alt+Y**) dan **Buka ke Catatan Dokumen** (**Ctrl+Alt+N**) memindahkan papan ketik ke sana dari teks; **Alt+2** kembali ke editor. Sejak 0.93.0 catatan (catatan dokumen dan proyek) dimulai sebagai teks biasa: tanpa font, ukuran, indentasi baris pertama, spasi paragraf, atau gaya dari pemformatan teks proyek, dalam font antarmuka program pada zoom catatan. Pemformatan muncul hanya ketika Anda menerapkannya sendiri (bilah format, menu, pintasan) dan kemudian tetap. Catatan yang sudah berisi teks mempertahankan tampilan yang dimilikinya. Teks yang ditempel ke catatan berupa teks biasa; **Edit > Tempel dengan Pemformatan** (**Ctrl+Alt+Shift+V**) mempertahankan pemformatan yang dibawanya.
- **Penanda**: penanda dokumen dan penanda proyek. Klik pada penanda sebuah dokumen proyek menampilkan teksnya di bawah daftar. Anda dapat menulis dan memformat di sana seperti di editor (bilah format, menu Format, dan pintasannya bekerja padanya selama kursor berada di dalamnya): semuanya langsung masuk ke dokumen dan berkasnya, dan editor yang menampilkan dokumen yang sama langsung menampilkan perubahannya. Teks di sana memiliki zoom sendiri.
- **Peta Pikiran**: papan catatan untuk dokumen atau folder (lihat "Peta pikiran" di bawah).
- **Objek**: tokoh, lokasi, lingkungan, tempat menarik, cuaca, dan hal-hal lain dalam cerita Anda, masing-masing dengan lembar sendiri dan tautan di antaranya (lihat "Objek" di bawah).
- **Metadata**: label, status, jenis bagian, sertakan dalam kompilasi, tanggal dibuat dan diubah, **metadata kustom**, dan **kata kunci**.
- **Snapshot** (bagian 11).
- **Komentar dan catatan kaki**.

Alat menulis dari pengaya dapat menambahkan tab sendiri; tombolnya muncul lebih dulu di barisnya, misalnya **Transkripsi** dari Transkripsi Ucapan (bagian 10).

**Zoom inspektor sendiri.** **Ctrl+roda mouse** di atas bagian inspektor membuat bagian itu lebih besar atau lebih kecil, tanpa mengubah editor. Tab dengan dua bagian memiliki zoom untuk masing-masing: **Sinopsis dan catatan** (kartu, catatan), **Penanda** (daftar penanda dokumen, daftar penanda proyek, teks penanda di bawahnya: tiga zoom), **Snapshot** (daftar, teks snapshot), **Objek** (tombol kategori dengan daftar, lembar). Daftar **Zoom** di bagian bawah menampilkan dan menetapkan zoom bagian yang memegang papan ketik, atau bagian yang terakhir Anda zoom; judulnya menyebutkan bagian itu ("Zoom: Catatan"), dan pada tab Peta Pikiran itu adalah zoom papan. Tab lainnya (Metadata, Komentar) serta baris tombol di bagian atas dan bawah berbagi satu zoom; dengan **Tampilan > Zoom Tab Inspektor Secara Terpisah** (juga di menu klik kanan daftar Zoom) Metadata dan Komentar masing-masing memiliki satu.

**Inspektor mengingat** bersama proyek tab mana yang terbuka, di mana Anda menaruh pembagi di dalam tab, penanda yang ditampilkan, dan objek yang ditampilkan pada tab Objek.

**Komentar sebuah folder.** Dengan folder di editor (sebagai papan gabus atau kerangka), tab komentar dan catatan kaki mencantumkan komentar dan catatan kaki setiap dokumen di dalam folder dan subfoldernya, dikelompokkan menurut dokumen. Klik pada nama dokumen membukanya; klik pada **Komentar ▸** (atau **Catatan kaki ▸**) membuka dokumen dan menampilkan tempat itu; teks komentar atau catatan kaki dapat diubah langsung di sana. Pada teks gabungan (beberapa dokumen sebagai satu teks), tab mencantumkan catatan semua dokumen yang ditampilkan.

### Peta pikiran

Setiap dokumen dan folder dapat memiliki **peta pikiran**: papan tempat Anda menaruh catatan di mana saja, menghubungkannya dengan garis dan panah, mewarnainya, dan mengelompokkannya pada bentuk latar belakang. Papan dapat disimpan sebagai berkas Scapple, dan berkas Scapple dapat dibaca (lihat *Berkas dan Scapple* di bawah). Klik tombol **Peta Pikiran** pada inspektor (di antara Penanda dan Metadata). Tombol itu bergaris bawah bila dokumen memiliki peta pikiran.

**Peta pikiran yang dikunci (0.92.0).** Biasanya tab menampilkan peta pikiran dokumen atau folder yang Anda pilih. Anda dapat mengunci tab pada satu item: tab kemudian terus menampilkan peta pikiran item itu, apa pun yang Anda pilih di binder atau buka di editor. Ini untuk papan yang ingin Anda dekatkan selagi menulis, seperti peta alur yang disimpan pada sebuah folder. Tiga tempat melakukan hal yang sama:

- **Di binder**, gerakkan mouse di atas sebuah baris: pin kecil muncul di dekat ujung kanannya. Klik, dan item itu dikunci; pinnya tetap dan terisi. Klik pada pin yang terisi melepaskannya. Mengklik pin tidak memilih apa pun.
- **Di tab Peta Pikiran**, tombol pin di sisi kanan bilah kecil mengunci papan yang ditampilkan tab dan melepaskannya lagi. Selama papan dikunci, bilah menyebutkan itemnya (*Dikunci: Plot*, tepat setelah tombol gambaran umum; judul yang panjang dipersingkat, dan seluruh judul tampil bila Anda mengarahkan mouse ke atasnya), sehingga jelas bahwa papan itu bukan milik dokumen yang dipilih.
- **Di menu**, **Navigasi > Inspeksi > Kunci Peta Pikiran Item Ini** dan **Navigasi > Inspeksi > Lepaskan Peta Pikiran yang Dikunci** (awalnya tanpa pintasan; berikan pintasan di opsi bila Anda suka).

Selama sebuah item dikunci, bilah ramping di bagian bawah binder menyatakan *Peta pikiran dikunci:* dengan judulnya dan memiliki tombol **Lepaskan**. Paling banyak satu item proyek yang dikunci; mengunci yang lain memindahkan kuncinya. Item tanpa peta pikiran juga dapat dikunci: tab menampilkan papan kosong untuk memulai. Hanya tab Peta Pikiran yang dikunci; tab inspektor lainnya mengikuti dokumen yang dipilih seperti biasa, dan papan yang dibuka di jendela besar tetap seperti adanya. Penguncian disimpan bersama proyek di komputer ini saat Anda menutupnya, dan dilepaskan ketika item masuk ke Tempat Sampah.

**Di inspektor** papan memenuhi tab. Bilah kecil di atasnya memiliki **Catatan Baru**, **Bentuk Latar Belakang Baru**, **Zoom agar Pas**, **Menu** (semua perintah: Berkas, Edit, Tampilan, Catatan, Format), tombol **gambaran umum** (lihat *Melihat sekeliling*), dan **Buka di Jendela**. Bilah di bawahnya menghitung catatan dan memiliki penggeser zoom. **Buka di Jendela** menampilkan papan yang sama di jendela besar dengan menu yang sama di bilah menunya dan inspektor di kanan (**Format Catatan**, **Gaya Catatan**, **Pengaturan Dokumen**); perubahan di satu tempat langsung tampak di tempat lain. Klik kanan pada papan membuka perintah untuk apa yang ada di bawah mouse. Catatan baru (tombol, menu, atau klik dua kali) langsung mengambil papan ketik, juga bila sebelumnya berada di binder: apa yang Anda ketik menggantikan teks yang dipilihnya (0.94.0).

Perintah peta pikiran ada di menu-menunya sendiri ini, bukan di menu jendela utama. Di bawah, **Menu > Catatan > Hubungkan** berarti: tombol **Menu** pada papan inspektor, lalu **Catatan**, lalu **Hubungkan** (di jendela besar: **Catatan > Hubungkan** pada bilah menunya). Selama papan memegang papan ketik, tombolnya didahulukan: **Ctrl+Z** mengurungkan perubahan papan, **Ctrl+F** mencari di catatannya, **Ctrl+P** mencetaknya.

**Bantuan untuk peta pikiran.** **Bantuan > Bantuan Peta Pikiran** (juga tombol **?** di ujung kanan bilah papan, dan **Menu > Bantuan** pada papan) menampilkan panduan singkat bergambar: cara membuat catatan, cara menggambar garis (seret catatan ke catatan lain), cara menggambar panah (tahan **Alt** saat Anda melepas; **Ctrl+Alt** untuk panah balik, **Ctrl+Alt+Shift** untuk kedua ujung), kata pada garis, dan bentuk latar belakang. Tombolnya **Buka Papan Contoh** (juga **Menu > Bantuan > Buka Papan Contoh**) membuka papan latihan di jendela tersendiri, dengan catatan untuk diseret ke satu sama lain dan contoh garis dan panah yang sudah jadi. Tidak ada apa pun di papan contoh yang menjadi milik proyek Anda, dan papan itu tidak disimpan.

**Catatan.**

- **Klik dua kali** pada tempat kosong: catatan baru, siap diketik. **Ctrl+Enter** membuat catatan baru di bawah yang dipilih (bertumpuk). Dengan **Ctrl** pada klik dua kali, catatan baru dihubungkan ke catatan yang dipilih; dengan **Alt**, oleh panah menuju ke sana; dengan **Ctrl+Alt**, oleh panah balik.
- **Klik dua kali** pada catatan untuk mengubah teksnya; **Esc** atau klik di luar mengakhiri pengeditan. Catatan pendek menyempit sesuai teksnya; yang panjang membungkus pada lebar catatan baru (keduanya di **Menu > Berkas > Opsi Peta Pikiran…**).
- **Seret** catatan untuk memindahkannya; dengan **Ctrl** saat dijatuhkan, catatan disalin. **Tombol panah** memindahkan catatan yang dipilih bila **Menu > Edit > Tombol Panah Memindahkan Catatan** (**M**) menyala (dengan **Shift** sebesar 10 px); jika tidak, tombol itu memilih catatan berikutnya ke arah itu. **Enter** atau **F2** mengedit catatan yang dipilih, **Delete** menghapus catatan yang dipilih.
- **Seret catatan ke catatan lain** untuk menghubungkan keduanya dengan garis; lagi untuk menghapus garis. Dengan **Alt** garis mendapat panah menuju catatan tempat ia dijatuhkan; dengan **Ctrl+Alt** panah balik; dengan **Ctrl+Alt+Shift** panah di kedua ujung.
- **Klik dua kali garis** untuk menaruh catatan baru di dalamnya, atau label garis untuk mengubahnya (**Menu > Catatan > Label Hubungan…**).
- **Menu > Catatan > Hubungkan** (**Ctrl+.**), **Hubungkan dengan Panah** (**Ctrl+Alt+.**), **Hapus Panah** (**Ctrl+,**), dan **Putuskan** bekerja pada catatan yang dipilih: yang pertama Anda pilih adalah pusatnya.
- **Menu > Catatan > Catatan Terhubung Baru** membuat catatan di kiri, kanan, atas, atau bawah yang dipilih, terhubung dengannya (dengan atau tanpa panah).
- **Menu > Catatan > Pisahkan** (**Ctrl+Shift+K**) membagi catatan pada kursor; **Gabungkan** menyatukan catatan yang dipilih.
- **Menu > Catatan > Tumpuk** (**Ctrl+'**), **Ratakan**, **Distribusikan**, **Samakan Lebar / Tinggi**, **Pas Otomatis**, **Bawa ke Depan**, **Kirim ke Belakang**.
- **Menu > Edit > Tambah Tautan…** (**Ctrl+K**) menautkan teks catatan ke halaman web atau berkas; dokumen yang diseret dari binder ke papan menjadi catatan dengan tautan ke dokumen itu. Klik pada tautan membukanya: dokumen terbuka seperti tautan dokumen di teks (**Berkas > Opsi > Perilaku**).
- **Menu > Catatan > Tambahkan Catatan ke Binder** mengubah catatan yang dipilih menjadi dokumen di dalam dokumen atau folder pemilik papan, dalam urutan baca papan.

**Bentuk latar belakang** (**Menu > Catatan > Bentuk Latar Belakang Baru**, **Ctrl+Shift+G**) adalah area berwarna di belakang catatan. Bila dibuat dengan catatan terpilih, bentuk itu mengelilinginya. Bentuk **magnetis** (**Menu > Catatan > Magnetis**, **Ctrl+Shift+M**) membawa catatan di atasnya ketika dipindahkan. Bentuk baru dimulai dengan abu-abu sedang (#808080) dan tidak magnetis; **Berkas > Opsi > Ekstra > Peta Pikiran** (dan **Menu > Berkas > Opsi Peta Pikiran…**) menetapkan isian, batas, dan **Magnetis** yang menjadi awal bentuk baru. (Sebelum 0.93.0 bentuk baru berwarna abu-abu pucat dan magnetis; warna yang telah Anda pilih di sana tetap, dan **Magnetis** yang tersimpan dimatikan satu kali.)

**Menu klik kanan sebuah catatan** (atau bentuk latar belakang) mengubah catatan yang dipilih: **Warna Isian**, **Warna Teks**, dan **Warna Batas** menawarkan warna label proyek Anda, keenam warna bilah format (merah, biru, kuning, hijau, putih, hitam), dan **Lainnya...** untuk warna apa pun, pilihan yang sama di mana pun dalam {{APP}}; **Bentuk** adalah **Persegi Panjang**, **Persegi Panjang Membulat**, **Awan**, atau **Bergerigi** (catatan tanpa batas mendapat batas tipis, agar bentuknya tampak); **Batas** menetapkan ketebalan; **Ikon** menaruh hingga delapan ikon pada sebuah catatan, satu di setiap sudut dan satu di tengah setiap sisi, dipilih dari ikon binder (**Dokumen > Ubah Ikon**), termasuk ikon Anda sendiri. Klik kanan di dekat tempat sebuah ikon untuk menghapus hanya yang itu; **Hapus Semua Ikon** membersihkan semuanya.

**Menu klik kanan sebuah garis** hanya milik garis itu, agar garis lain dari catatan yang sama tidak berubah: panahnya (**Tanpa Panah**, panah ke salah satu catatan, atau **Panah di Kedua Ujung**), **Warna Garis**, **Ikon di Tengah** (ikon yang digambar di tengah garis), label, **Sisipkan Catatan pada Garis**, dan **Putuskan**. Setiap perubahan dapat diurungkan sendiri-sendiri.

Ikon, warna garis, dan ikon garis adalah tambahan milik {{APP}} sendiri; semuanya disimpan di berkas papan dalam elemen yang tidak dibaca Scapple (papan yang diekspor sebagai berkas Scapple memiliki catatan, garis, dan panah tanpa itu). **Berkas > Opsi > Ekstra > Peta Pikiran** menetapkan ukuran ikon dan bentuk catatan baru.

**Tampilan.** **Menu > Format** memiliki tebal, miring, garis bawah, coret, lebih besar dan lebih kecil (**Ctrl+Shift+>** dan **Ctrl+Shift+<**), warna teks, isian, dan batas, ketebalan batas (tanpa, 1 hingga 5 px) dan gaya (membulat, persegi, bergerigi, awan), bayangan, dan **Pudarkan** (**Ctrl+Shift+F**, catatan ditampilkan pucat). **Gaya Catatan** menyimpan tampilan dengan sebuah nama; **Menu > Format > Gaya Catatan** mencantumkan gaya catatan (satu klik menerapkannya; 0.99.0), lalu **Gaya Catatan Baru dari Pilihan**, **Definisikan Ulang Gaya Catatan dari Pilihan…** dan **Hapus Gaya Catatan…** (kedua perintah terakhir menanyakan gaya yang mana); menu ini menerapkan satu gaya (**Ctrl+Alt+1** hingga **9**, **Ctrl+Alt+0** untuk Bawaan), membuat satu dari catatan yang dipilih, atau menyalin dan menempel tampilan sebuah catatan (**Alt+Shift+C** / **Alt+Shift+V**). **Pengaturan Dokumen** di inspektor jendela menetapkan latar belakang, warna teks, font, dan arah penulisan papan (dan **Tetapkan sebagai Bawaan** untuk papan baru).

**Latar belakang.** Klik kanan pada tempat kosong di papan (di inspektor atau di jendela), **Menu > Format > Warna Latar Belakang**, atau tombol latar belakang **Pengaturan Dokumen** menawarkan warna label proyek Anda, keenam warna bilah format, bawaan (**Bawaan untuk Tema Terang** atau **Bawaan untuk Tema Gelap** di menu, **Pakai Bawaan** di Pengaturan Dokumen), dan **Lainnya...**. Papan memiliki latar belakang sendiri untuk tema terang dan satu untuk tema gelap: warna yang Anda pilih disimpan untuk jenis tema yang sedang dipakai, dan jenis yang lain mempertahankan miliknya. Papan tanpa warna sendiri menampilkan bawaan **Berkas > Opsi > Ekstra > Peta Pikiran** ("Latar belakang papan", satu untuk tema terang dan satu untuk tema gelap) dan mengikutinya ketika Anda mengubahnya. **Tetapkan sebagai Bawaan** di Pengaturan Dokumen menjadikan latar belakang papan sebagai bawaan jenis tema yang sedang dipakai. Papan mempertahankan warna latar belakangnya juga ketika belum ada apa pun di atasnya.

**Zoom dan tempat.** Setiap papan mengingat, bersama proyek, zoomnya dan bagian yang ada di layar, satu untuk inspektor dan satu untuk jendela besar. Papan yang belum pernah Anda zoom terbuka pada zoom bawaan peta pikiran (**Berkas > Opsi > Pengeditan**, "Zoom teks bawaan").

**Memilih.** Klik, **Shift+klik** atau **Ctrl+klik** untuk menambah, seret bingkai di sekitar catatan, **Ctrl+A**. **Menu > Edit > Pilih** memiliki catatan yang bertumpuk di bawah, catatan pada bentuk yang dipilih (**Alt+A**), catatan terhubung (**Ctrl+Shift+A**), dan seluruh gugus terhubung; di bawahnya ada **Balik Pilihan** (**Alt+Shift+I**) dan **Batalkan Pilihan** (**Ctrl+D**).

**Melihat sekeliling.** Zoom dengan **Ctrl+roda mouse**, **Ctrl++** dan **Ctrl+-**, **Ukuran Sebenarnya** (**Ctrl+0**), **Zoom agar Pas** (**Ctrl+9**), **Zoom ke Pilihan** (**Ctrl+Shift+8**), dan **Zoom Acak** (**Ctrl+Shift+R**). Tahan **Spasi** dan seret untuk menggeser papan; tahan **Z** untuk melihat seluruh papan sejenak. **Menu > Edit > Cari** (**Ctrl+F**) mencari dan mengganti teks di catatan. **Menu > Tampilan > Pemandu Halaman** menunjukkan di mana halaman berakhir saat mencetak.

**Gambaran umum (0.93.0).** Peta kecil di sudut kiri atas papan menampilkan seluruh papan dalam ukuran kecil, dan bingkai untuk bagian yang Anda lihat sekarang; bingkai bergerak dan berubah ukuran saat Anda menggulir dan melakukan zoom. Catatan, bentuk latar belakang, dan garis digambar dengan warnanya; teks catatan hanya ditampilkan di tempat ia masih dapat dibaca pada ukuran itu, jika tidak catatan itu hanyalah bingkai berwarnanya. Klik di gambaran umum untuk memindahkan tampilan ke titik itu; seret bingkai untuk menggeser tampilan bersamanya. Zoom tetap, dan papan yang dikunci (**Cegah Pengeditan**) juga dapat digeser. Roda mouse di atas gambaran umum bekerja seperti di atas papan (**Ctrl+roda** melakukan zoom). Seret pegangan di sudut kanan bawahnya untuk mengubah lebarnya (awalnya seperlima lebar tampilan, minimal 80 px, dan tidak pernah lebih lebar dari setengah tampilan); tingginya mengikuti bentuk apa yang ditampilkannya, tidak pernah lebih dari setengah tampilan, dan lebarnya menyusut bersamanya bila tinggi akan melampaui itu. Gambaran umum hanya milik tampilan: tidak ada di ekspor, cetakan, atau gambar papan. Awalnya **otomatis**: tampil ketika sebagian papan berada di luar tampilan, dan bersembunyi ketika semuanya muat (dengan margin kecil, agar tidak berkedip di tepi). Tombol **gambaran umum** di bilah papan inspektor (tepat setelah **Menu**) beralih antara **otomatis** dan **selalu ditampilkan** (tombol tampak tertekan); sakelar yang sama adalah entri **Gambaran Umum Selalu Ditampilkan** di menu Tampilan papan (di inspektor: tombol **Menu**, lalu **Tampilan**; di jendela besar: bilah menunya). Pilihan dan ukurannya disimpan di komputer ini, di pengaturan program (satu pilihan untuk semua papan; ukuran gambaran umum inspektor dan jendela besar terpisah); tidak ada yang masuk ke proyek.

**Teks, gambar, dan berkas.** Tempel atau jatuhkan teks untuk membuat catatan (teks dengan baris kosong dapat menjadi beberapa catatan), jatuhkan gambar untuk catatan gambar, dan jatuhkan berkas teks untuk membuat catatan dari paragrafnya.

**Berkas dan Scapple.** **Menu > Berkas > Ekspor** menulis papan sebagai PDF (**Ctrl+Shift+E**), gambar PNG, teks biasa, daftar teks, RTF, kerangka OPML, gambar-gambarnya, atau berkas Scapple (*.scap*); **Menu > Berkas > Impor Peta Pikiran atau Berkas Scapple…** membaca berkas *.scap* ke dalam papan. **Cetak** dan **Pratinjau Cetak** mencetaknya. **Menu > Berkas > Cegah Pengeditan** mengunci papan.

**Opsi Peta Pikiran** (**Menu > Berkas > Opsi Peta Pikiran…**, dan yang sama di **Berkas > Opsi > Ekstra > Peta Pikiran** pada jendela utama): apakah **Enter** mengakhiri pengeditan catatan (maka **Shift+Enter** membuat baris baru), apakah **Enter** pada catatan terpilih membuat yang baru, apakah catatan baru mengambil tampilan catatan yang dipilih, garis putus-putus tanpa panah, catatan sempit untuk teks pendek, seberapa pucat catatan yang dipudarkan, serta ukuran, lebar, warna, dan batas catatan dan bentuk baru.

**Di mana papan disimpan.** Papan disimpan dengan sendirinya, sebagai berkas dalam tata letak berkas Scapple (*content/<ID>/mindmap.scap* di folder proyek; *Files/Data/<UUID>/quillbinder-mindmap.scap* di proyek Scrivener, di samping berkas milik dokumen). Dokumen mendapat berkas itu hanya ketika sesuatu ditaruh di papannya. Urungkan (**Ctrl+Z**) bekerja untuk setiap papan secara tersendiri.

{{APP}} membaca berkas Scapple seperti yang ditulis Scapple 1.x, sejauh tata letaknya diketahui; berkas-berkas itu tidak dapat diuji dengan Scapple sendiri (lihat catatan versi 0.17.0 di berkas status).

### Objek: tokoh, lokasi, dan lainnya

**Objek** adalah orang, tempat, dan benda dalam cerita Anda, masing-masing dengan lembar berisi kolom: tokoh dengan ras, tinggi badan, warna mata, sifat, dan latar belakang cerita; tempat dengan jenisnya, lingkungannya, dan cuacanya; dan seterusnya. Objek milik seluruh proyek, bukan satu dokumen, dan disimpan terpisah dari binder. Tautan menghubungkannya, sehingga Anda dapat menelusuri sebuah latar belakang dengan klik: lembar tokoh, desa tempat ia tinggal, lanskap di sekitarnya, cuacanya.

Klik tombol **Objek** pada inspektor (di kanan tombol **Peta Pikiran**), atau pilih **Navigasi > Inspeksi > Objek**. **Jendela > Objek** (atau tombol jendela di bilah tab) menampilkan objek yang sama di jendela besar tersendiri, dengan daftar di kiri dan lembar di kanan. Perubahan di satu tempat langsung tampak di tempat lain.

**Kategori.** Tombol berwarna di bagian atas adalah kategori: **Tokoh**, **Lokasi**, **Lingkungan**, **Tempat Menarik**, dan **Cuaca** sebagai awal. Klik salah satu untuk melihat objeknya. Anda dapat mengganti namanya, memberinya warna lain, mengubah urutannya, menghapusnya, dan menambah kategori sendiri (faksi, mantra, kapal ...): lihat *Templat dan warna* di bawah.

**Membuat objek.** **+** di bilah tab membuat objek baru dalam kategori yang dipilih (di folder objek yang dipilih); ketik namanya di bagian atas lembar. Tombol dengan folder membuat **folder** untuk menyortir, misalnya "Villages" di dalam Lokasi; folder dapat memuat folder. Seret objek dan folder di daftar untuk memindahkannya. **F2** atau klik kanan mengganti nama, dan menu klik kanan juga memiliki **Duplikat** dan **Hapus…** (juga di menu **≡** pada bilah). Kolom **Filter** hanya menampilkan objek yang nama atau teksnya memuat apa yang Anda ketik.

**Lembar.** Di bawah garis berwarna dengan kategori dan nama terdapat kolom-kolom templat kategori. Semua yang Anda ketik disimpan dengan sendirinya, seperti dokumen Anda. Jenis kolomnya adalah:

- **Teks pendek** (satu baris) dan **Tanggal** (dalam bentuk apa pun yang Anda suka, seperti "spring 1203" atau "the third winter").
- **Angka** dengan satuan (cm, tahun, °C ...).
- **Teks panjang**, yang dapat memuat tautan (lihat di bawah), serta kata **tebal**, *miring*, bergaris bawah, tercoret, berwarna, dan tersorot: dengan kursor di kolom, bilah format, menu Format, dan pintasannya (**Ctrl+B**, **Ctrl+I**, **Ctrl+U**; juga di jendela Objek) bekerja padanya. Versi 0.20 dan 0.21 {{APP}} juga menampilkannya.
- **Pilihan**: daftar untuk dipilih; Anda juga dapat mengetik hal lain.
- **Ya / tidak**.
- **Gambar**: klik area (atau jatuhkan gambar ke atasnya) untuk memilih; gambar disalin ke proyek, diperkecil bila lebih besar dari 1600 piksel. Klik kanan menghapusnya.
- **Tautan ke objek**: objek yang ditautkan ditampilkan sebagai tombol berwarna. **+** menambahkan satu: ketik untuk mencari, lalu pilih objek, atau pilih **Baru di Lokasi: "Ashvale"** untuk membuat objek baru dengan nama itu dan langsung menautkannya. Klik pada objek yang ditautkan menampilkan lembarnya; klik kanan menghapus tautan.
- **Tautan ke dokumen**: dokumen binder yang menjadi milik objek, seperti "Muncul di" milik tokoh atau "Adegan yang berlatar di sini" milik tempat. Klik membuka dokumen di editor.

**Tombol warna.** Tombol **Warna** pada garis berwarna memberi objek warna tersendiri (atau mengembalikan warna kategorinya), misalnya untuk menandai tokoh utama.

**Tautan di dalam teks panjang.** Di kolom teks panjang, pilih sebuah kata dan pilih **Tautan ke Objek…** (atau **Tautan ke Dokumen…**) di menu klik kanannya; tanpa pilihan, nama objek dimasukkan sebagai tautan. Klik pada tautan mengikutinya; **Hapus Tautan** di menu klik kanan menghapusnya lagi. Di teks dokumen, **Sisipkan > Tautan ke Objek…** menaruh tautan ke objek di dalam teks; klik padanya menampilkan objek di inspektor. Ketika naskah dikompilasi, tautan-tautan ini menjadi kata biasa.

**Maju mundur.** **‹** dan **›** di bilah tab bergerak mundur dan maju di antara objek yang Anda lihat, seperti peramban web.

**Ditautkan dari.** Di bagian bawah setiap lembar, **Ditautkan dari** mencantumkan objek yang menautkan ke objek ini (di kolom tautan atau di teks), sehingga desa menunjukkan siapa yang tinggal di dalamnya. Klik menuju ke sana.

**Di dokumen ini.** Ketika dokumen di editor ditautkan ke objek (melalui kolom tautan dokumen, misalnya "Muncul di"), tab menampilkan **Di dokumen ini:** dengan objek-objek itu, dan tombol **Objek** bergaris bawah seperti tab lain yang berisi.

**Menghapus.** Menghapus objek bertanya lebih dulu. Tautan ke objek itu dihapus dari objek lain (di teks, kata yang ditautkan tetap, sebagai teks biasa). Tautan ke objek itu di dokumen menyatakan bahwa objek tidak ada lagi ketika diklik. Menghapus folder mempertahankan objeknya; objek naik satu tingkat.

**Templat dan warna.** **Proyek > Templat dan Warna Objek…** (juga di menu **≡** tab, dan di menu klik kanan tombol kategori) mengedit kategori dan kolomnya:

- Di kiri, kategori: **Tambah**, **Hapus**, dan panah untuk urutan. Menghapus kategori yang memiliki objek menanyakan apakah objeknya juga akan dihapus; **Tidak** tidak mengubah apa pun.
- Di kanan, nama dan **Warna** kategori, serta kolomnya: **Tambah Kolom**, **Hapus**, **Naik**, **Turun**. Untuk setiap kolom, nama dan jenisnya; angka dapat memiliki satuan, pilihan memiliki daftar pilihannya (satu per baris), dan kolom tautan memiliki kategori yang dapat ditautkannya (tidak ada yang dicentang: semua).
- **Pakai untuk Proyek Baru** menyimpan kategori dan templat ini untuk proyek yang Anda buat mulai sekarang. **Templat Standar** mengatur ulang kategori ke templat bawaannya, atau menambahkan kategori standar yang Anda hapus.
- Menghapus kolom dari templat menyembunyikan apa yang dimiliki objek di dalamnya, tetapi tidak menghapusnya dari berkasnya. Untuk kolom standar, **Templat Standar > Atur Ulang Kategori Ini ke Standar** mengembalikannya beserta isinya; kolom buatan Anda sendiri tidak kembali, jadi gantilah nama kolom daripada menghapusnya bila Anda mungkin menginginkan isinya lagi.

**Di mana objek disimpan.** Di folder proyek, dalam folder *objects*: satu berkas kecil per objek (*objects/characters/<ID>.json* dan seterusnya), templat, warna, dan folder di *objects/schema.json*, dan gambar di *objects/images*. Berkas proyek itu sendiri tidak diubah, sehingga versi {{APP}} yang lebih lama membuka proyek seperti sebelumnya dan membiarkan objeknya. Proyek dalam format Scrivener menyimpan objeknya di salinan kerjanya (bagian 14); Scrivener tidak mengenalnya. Cadangan menyertakannya.

---

## 8. Mengatur proyek Anda

### Kata kunci

Kata kunci adalah tag, misalnya tokoh atau tempat dalam sebuah adegan. Tambahkan di inspektor atau kelola semuanya dengan **Proyek > Kata Kunci Proyek** (**Ctrl+Alt+K**), tempat Anda dapat memberinya warna, menetapkannya ke dokumen yang dipilih, dan menemukan semua dokumen dengan sebuah kata kunci. Anda juga dapat **menyeret kata kunci** dari jendela itu ke dokumen di binder, koleksi, papan gabus, atau kerangka (bila dokumen itu salah satu dari beberapa yang dipilih, semuanya mendapat kata kunci), atau ke daftar kata kunci inspektor atau panel Referensi Cepat. Tahan **Alt** saat menjatuhkan untuk menetapkan juga grup kata kunci itu. Di dalam jendela, seret kata kunci ke kata kunci lain untuk memasukkannya ke grup itu. Warna kata kunci muncul pada kartu indeks.

### Metadata kustom

Di bawah **Proyek > Pengaturan Proyek > Metadata Kustom** Anda dapat menambahkan kolom sendiri: teks, kotak centang, daftar (sekumpulan pilihan tetap), atau tanggal, misalnya *POV*, *Location*, atau *Story Day*. Kolom itu muncul di inspektor dan sebagai kolom kerangka.

### Menyimpan pengaturan proyek ke berkas

**Proyek > Simpan Pengaturan Proyek ke Berkas…** menulis pengaturan proyek yang terbuka ke berkas tersendiri (*.qbsettings*): label, status, jenis bagian, metadata kustom, kata kunci, pemformatan bawaan, gaya, format skenario, target, pengaturan kompilasi, tampilan halaman dan ukuran halaman, pengaturan cetak, tampilan (tema, warna, font, lebar kolom, batas), opsi milik proyek sendiri (Berkas > Opsi dengan "Proyek ini"), serta templat dan warna lembar Objek. Teks dokumen Anda bukan bagian darinya. Label, jenis bagian, dan folder yang disebut pengaturan kompilasi disimpan menurut namanya, sehingga berkas dapat dibaca ke proyek lain.

**Proyek > Muat Pengaturan Proyek dari Berkas…** membaca berkas seperti itu ke proyek yang terbuka. Bila pengaturan proyek sama dengan yang ada di berkas, program menyatakannya dan tidak mengubah apa pun. Bila berbeda, jendela mencantumkan setiap bagian beserta perbedaannya (misalnya "hanya di berkas: Idea; hanya di proyek: Revise") dan memungkinkan Anda memilih untuk setiap bagian:

- **Ambil dari Berkas**: pengaturan berkas untuk bagian ini.
- **Pertahankan Milik Proyek**: bagian tetap seperti adanya.
- **Bawaan Quillbinder**: apa yang didapat proyek baru (pengaturan proyek baru di **Berkas > Opsi**, label dan status standar ...).

**Ambil Semua dari Berkas** dan **Pertahankan Semua Milik Proyek** menetapkan setiap bagian sekaligus. Apa pun yang Anda pilih, tidak ada yang dipakai dokumen Anda yang hilang: label, status, jenis bagian, kata kunci, kolom metadata kustom, kategori objek, atau gaya yang masih dipakai dokumen tetap berada di proyek meskipun berkas tidak memilikinya, dan pesan di akhir menyebutkan apa yang dipertahankan. Ketika pemformatan atau gaya berubah, teks semua dokumen mengikutinya, seperti ketika Anda mengubahnya sendiri. Sebelum memuat, {{APP}} menyimpan pengaturan seperti sebelumnya dalam berkas di folder *settings* proyek ("settings-before-loading-*date*.qbsettings"); muat berkas itu untuk kembali.

### Jenis bagian

Jenis bagian memberi tahu Kompilasi apa setiap dokumen itu: *Part*, *Chapter*, *Scene*, *Section*, dan seterusnya. Tetapkan per dokumen di inspektor, atau biarkan {{APP}} menentukan menurut posisi (**Pengaturan Proyek > Jenis Bagian**, "Jenis bawaan menurut struktur"), misalnya: folder di tingkat pertama adalah bab, dokumen teks di dalamnya adalah adegan.

### Templat dokumen

Taruh dokumen templat (misalnya lembar tokoh) di sebuah folder, lalu pilih folder itu di **Pengaturan Proyek > Folder Khusus**. Templat muncul di **Proyek > Baru dari Templat**. Di **Dokumen > Templat Bawaan untuk Subdokumen** Anda dapat membuat sebuah folder memakai templat untuk setiap dokumen baru yang Anda buat di dalamnya.

### Templat proyek

**Berkas > Simpan sebagai Templat…** menyimpan proyek saat ini (struktur, pengaturan, dan dokumen contoh apa pun) sebagai templat untuk proyek baru.

---

## 9. Mencari

### Pencarian cepat

Ketik beberapa huruf ke kolom di bilah alat. Daftar menampilkan dokumen yang **judul**, **sinopsis**, atau **teks**-nya memuatnya, hingga sepuluh dari setiap jenis di bawah judulnya sendiri. Pilih satu untuk membukanya: tempat-tempat di teks yang memuat apa yang Anda ketik **ditandai**, dan yang pertama dipilih serta digulir ke tampilan. Tanda hilang ketika Anda memilih dokumen lain di binder. Ini cara tercepat menuju dokumen yang namanya Anda ketahui.

### Pencarian proyek

**Ctrl+Shift+F**, atau kaca pembesar di bilah alat, menampilkan kolom pencarian di atas binder. Ketik, dan binder digantikan oleh daftar hasil berwarna ungu.

- Klik hasil untuk membukanya. **Kecocokan disorot di teks**, yang pertama dipilih dan digulir ke tampilan, dan **F3** / **Shift+F3** melompat dari satu ke berikutnya. **Ctrl+F** membuka bilah pencarian yang sudah terisi dengan istilah pencarian Anda.
- **X** di header hasil kembali ke binder; editor mempertahankan dokumen yang ditampilkannya. Klik kaca pembesar di bilah alat lagi untuk menyembunyikan kolom pencarian.
- Dengan koleksi ditampilkan, tab **Hasil Pencarian** menjalankan pencarian terakhir Anda lagi. Pencarian terakhir disimpan bersama proyek.

Kaca pembesar di sisi kiri kolom pencarian membuka opsi:

| Kelompok | Entri |
|-------|---------|
| **Cari di** | **Semua**, atau salah satu dari: Judul, Teks, Sinopsis, Catatan, Kata Kunci, Label, Status, Metadata Kustom. Klik memilih satu tempat itu. Tahan **Alt** saat mengklik (atau saat membuka menu) untuk memilih beberapa tempat. |
| **Operator** | **Semua Kata** (setiap kata harus muncul, dalam urutan apa pun), **Kata Mana Pun**, **Frasa Tepat**, **Ekspresi Reguler** |
| **Opsi** | Kata Utuh, Peka Huruf Besar/Kecil, Balikkan Hasil (dokumen yang *tidak* cocok), Cari di Draf Saja, Cari di Pilihan Binder Saja (dokumen yang dipilih di binder dan semua yang ada di dalamnya), Cari Dokumen "Disertakan" / Cari Dokumen "Dikecualikan" (menurut **Sertakan dalam Kompilasi**), Sertakan Tempat Sampah |

Contoh: untuk melihat semua yang masih berstatus "To Do", cari `To Do` dan pilih **Status** di Cari di.

**Simpan Pencarian sebagai Koleksi…** di bagian bawah menu menyimpan pencarian, beserta opsinya, sebagai koleksi yang berjalan lagi setiap kali Anda membukanya.

### Ganti di Proyek

**Edit > Cari > Ganti di Proyek** mengganti teks di seluruh proyek sekaligus.

1. Masukkan teks untuk **Diganti** dan teks yang menggantikannya (**Dengan**). Biarkan "Dengan" kosong untuk menghapus teks.
2. Pilih **Cocokkan huruf besar/kecil**, **Hanya kata utuh**, atau **Ekspresi reguler** (maka "Dengan" boleh memakai `\1` hingga `\9` untuk grup ekspresi).
3. Centang di mana akan mengganti: **Judul**, **Teks**, **Catatan**, **Sinopsis**, **Metadata kustom** (kolom teks), **Komentar dan catatan kaki** (teks yang ditampilkan di inspektor).
4. Pilih **Seluruh proyek** atau **Hanya dokumen terpilih** (dokumen yang dipilih di binder, beserta isinya). Tempat Sampah dibiarkan kecuali Anda mencentangnya.
5. **Hitung** memberi tahu berapa banyak kemunculan yang akan berubah dan di mana. **Ganti Semua** bertanya sekali lagi lalu melakukannya.

Ganti di Proyek tidak dapat diurungkan secara keseluruhan. Biarkan **Buat snapshot setiap teks sebelum diubah** tercentang: setiap teks yang berubah kemudian memiliki snapshot bernama "Before replacing …" untuk dikembalikan (bagian 11). Judul dan sinopsis tidak memiliki snapshot; untuk perubahan besar, buat cadangan lebih dulu dengan **Berkas > Cadangkan > Cadangkan Sekarang**. Teks yang terbuka di editor juga dapat dibatalkan di sana dengan **Edit > Urungkan**.

Pemformatan tetap seperti adanya: teks baru mengambil pemformatan teks yang digantikannya. Teks yang tidak terbuka di editor diubah di berkasnya, sehingga **Edit > Urungkan** tidak membatalkan perubahan itu (snapshot yang dibuat lebih dulu bisa); hal yang sama berlaku untuk Konversi Tanda Kutip, kartu komentar, dan Bandingkan dan Transfer.

### Cari menurut Pemformatan

**Edit > Cari > Cari menurut Pemformatan** (**Ctrl+F3**) membuka jendela kecil yang tetap terbuka selagi Anda bekerja. Pilih apa yang dicari:

| Cari | Menemukan | Pilihan tambahan |
|------|-------|--------------|
| Teks Tersorot | teks yang ditandai dengan penyorot | batasi pada satu warna |
| Komentar dan Catatan Kaki | teks dengan komentar atau catatan kaki terlampir (inspektor) | batasi pada satu warna komentar |
| Anotasi Sebaris, Catatan Kaki Sebaris | catatan yang diketik di dalam teks | warna anotasi |
| Warna Revisi | teks yang diketik dalam mode revisi | revisi yang mana |
| Teks Berwarna | teks yang warnanya Anda tetapkan secara manual | batasi pada satu warna |
| Tautan | tautan web dan tautan ke dokumen | |
| Format Karakter | teks tebal, miring, bergaris bawah, tercoret (ditetapkan secara manual, bukan oleh gaya) | yang mana |
| Gaya | teks dengan gaya paragraf atau karakter | gaya yang mana |
| Gambar | gambar di dalam teks | |

**Berisi teks** mempersempit pencarian ke tempat-tempat yang memuat sebuah kata (untuk komentar dan catatan kaki, teks catatan itu sendiri juga dihitung). **Berikutnya** dan **Sebelumnya** memilih tempat berikutnya. Dengan **Cari di: Semua Dokumen**, pencarian berlanjut di dokumen-dokumen berikutnya di binder dan membukanya; dengan **Hanya Editor Saat Ini** pencarian tetap di tempat Anda berada. **Ctrl+G** dan **Ctrl+Shift+G** mengulang pencarian tanpa jendela. **Tutup** (atau **Esc**) menutup jendela.

---

## 10. Target, statistik, dan alat menulis

- **Proyek > Target Proyek** (**Ctrl+Shift+T**): tetapkan target jumlah kata atau karakter untuk Draf dan untuk setiap sesi menulis, serta tenggat dan hari menulis Anda jika perlu. {{APP}} kemudian dapat menghitung berapa banyak yang perlu Anda tulis pada setiap sesi. Footer menampilkan kemajuan Anda di samping jumlah kata (sejak 0.94.0): bilah kecil dan, misalnya, "83 / 1,000 hari ini" untuk sesi serta "12,400 / 80,000 draf" untuk Draf, masing-masing hanya selama targetnya ditetapkan; mengekliknya membuka jendela ini. Dua bilah tipis di bawah kotak pencarian cepat (biru: Draf; oranye: sesi) tetap ada, dan **Berkas > Opsi > Penampilan > Antarmuka Umum** dapat mematikannya. Kata yang masuk ke proyek sekaligus dalam satu langkah tidak dihitung sebagai kata yang ditulis dalam sesi (0.94.0): teks yang diimpor (**Berkas > Impor**, **Impor dan Pisahkan**, berkas yang dijatuhkan, halaman web), tempelan lebih dari 200 kata, dan kata yang sudah ada di proyek saat Anda membukanya untuk pertama kali. **Atur Ulang Sesi Sekarang** memulai hitungan dari 0, dan tooltip kotak pencarian cepat langsung menampilkannya. Jendela target menyisakan ruang untuk bilah-bilahnya, sehingga kolom isian tidak bergeser saat Anda menetapkan target.
- **Target dokumen**: klik target di footer editor.
- **Target pemangkasan** (0.95.0): target boleh berupa angka negatif, di setiap kolom target (Draf, sesi, dan dokumen). Itu adalah target untuk memangkas: "-2.000" untuk sesi berarti memangkas 2.000 kata hari ini, "-4.000" untuk dokumen berarti menurunkannya menjadi 4.000 kata. Bilahnya terisi saat jumlah kata turun (bilah sesi menurut kata yang dihapus, bilah dokumen atau Draf sampai jumlahnya turun ke angka itu), dan angka negatif ditampilkan dengan warna merah. Dengan target sesi negatif, hitungan hari ini bersifat bersih: kata yang dihapus dihitung sebagai angka di bawah 0, dengan warna merah; dengan target positif hitungan tetap seperti sebelumnya dan tidak pernah di bawah 0. Scrivener tidak menerima target negatif: target negatif hanya disimpan di salinan proyek milik {{APP}} sendiri dalam format Scrivener; berkas *.scriv* mendapat 0 ("tanpa target") untuknya.
- **Proyek > Statistik Proyek**: kata dan karakter di Draf, di teks hasil kompilasi, dan di seluruh proyek, ditambah perkiraan jumlah halaman. Pada proyek besar, penghitungan memerlukan waktu sejenak; penunjuk sibuk menandakannya.
- **Proyek > Riwayat Penulisan**: berapa banyak kata yang Anda tulis setiap harinya.

Alat menulis di bawah ini adalah pengaya (bagian 17): **Generator Nama**, **Catatan Sementara** dan **Statistik Teks** (ketiganya hadir bersama pembaruan 0.11 sebagai satu paket) serta **Transkripsi Suara** (0.61); masing-masing dapat dimatikan atau dihapus di **Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya**. Tanpa pengaya tersebut, **Edit > Alat Menulis** akan memberi tahu hal itu; Pengelola Pengaya ada di menu Bantuan.

### Statistik Teks

**Proyek > Statistik Teks…** menghitung sebagian dari proyek Anda. Pilih teks yang dihitung di bagian atas:

- **Teks terpilih** (dipilih saat Anda membuka jendela dengan teks yang sedang dipilih),
- **Teks di editor** (pada teks gabungan: semua dokumen yang ditampilkan),
- **Dokumen yang dipilih di binder**, beserta dokumen di dalamnya,
- **Draf (disertakan dalam kompilasi)**, atau **Seluruh proyek** (tanpa Tempat Sampah).

Jendela ini memiliki empat tab:

- **Ikhtisar**: kata (dihitung persis seperti bilah footer dan target), karakter dengan dan tanpa spasi, paragraf, kalimat, kata yang berbeda, halaman dengan 250 kata per halaman, waktu baca dengan 230 kata per menit, dan rata-rata (huruf per kata, kata per kalimat dan per paragraf, kalimat terpanjang; arahkan penunjuk ke kalimat itu untuk membacanya).
- **Keterbacaan (perkiraan)**, juga di tab pertama. Untuk bahasa Inggris: *Flesch Reading Ease* (0 sampai 100, makin tinggi makin mudah; 60 sampai 70 berarti bahasa Inggris yang lugas) dan *Flesch-Kincaid Grade Level* (tingkat kelas sekolah di AS yang diperlukan untuk mengikuti teks). Untuk bahasa Jerman: *indeks Flesch menurut Amstad* (skala yang sama) dan *Wiener Sachtextformel* (kira-kira setara tingkat kelas sekolah, 4 mudah sampai 15 sangat sulit). Untuk semua bahasa: *LIX* (kata per kalimat ditambah persentase kata yang lebih dari enam huruf; di bawah 30 sangat mudah, di atas 60 sangat sulit). Bahasa dikenali secara otomatis atau dipilih di bagian atas. Skor ini hanya panduan, bukan vonis: suku kata dan kalimat ditentukan dengan aturan praktis, dan rumus-rumus ini dibuat untuk prosa faktual, bukan untuk dialog atau puisi.
- **Frekuensi Kata**: setiap kata beserta jumlah dan persentasenya, dari yang paling sering muncul. **Sembunyikan kata umum** menyisihkan artikel, kata ganti, preposisi, dan sejenisnya (bahasa Inggris dan Jerman). Tetapkan panjang minimum, cari sebuah kata, atau hitung huruf kapital secara terpisah. Klik ganda sebuah kata untuk mencarinya di proyek.
- **Frasa Berulang**: kelompok yang terdiri dari dua sampai enam kata yang Anda gunakan lebih dari sekali di dalam kalimat, seperti deskripsi yang Anda tulis dua kali. Frasa yang hanya terdiri dari kata-kata umum disisihkan. Klik ganda untuk mencari di proyek.
- **Dokumen**: tabel dokumen dengan kata, karakter, kalimat, kata per kalimat, dan persentasenya terhadap keseluruhan. Klik judul kolom untuk mengurutkan; klik ganda sebuah dokumen untuk membukanya.

**Ekspor Tabel…** menyimpan tabel pada tab saat ini sebagai berkas CSV untuk lembar kerja.

### Generator Nama

**Edit > Alat Menulis > Generator Nama…** membuat nama untuk tokoh.

1. Centang daftar untuk **nama depan** (62 asal dari Eropa, Amerika, Asia, dan Afrika, misalnya Inggris, Amerika, Irlandia dan Gaelik Irlandia, Jerman, Austria, Swiss, Belanda, Prancis, Prancis Kanada, Italia, Spanyol, Meksiko, Portugis, Brasil, Denmark, Swedia, Norwegia, Finlandia, Islandia, Polandia, Ceko, Hungaria, Rusia, Yunani, Turki, Ibrani, Arab, Persia, India, Jepang, Tiongkok, Korea, Yoruba, Swahili, dan Zulu; belum ada daftar Skotlandia atau Wales). **Nama belakang** diambil dari daftar yang sama dengan nama depan, atau hapus centang **Daftar nama depan** lalu pilih daftar lain (nama depan Italia dengan nama belakang Irlandia).
2. Pilih **Jenis kelamin**, **Jumlah nama**, dan apa yang akan **Ditampilkan** (nama lengkap, hanya nama depan, hanya nama belakang, inisial dan nama belakang). **Dengan nama tengah** menambahkan nama depan kedua.
3. **Penyetelan halus**: **umum ↔ langka** memilih dari nama yang paling umum (kiri) atau yang langka (kanan); ini bekerja pada daftar yang ditandai *menurut frekuensi* (misalnya Amerika, Swedia, Hungaria, Ibrani), yang menyertakan seberapa sering setiap nama muncul. **Aliterasi** memberi persentase nama tersebut nama belakang dengan huruf pertama yang sama dengan nama depannya (Peter Parker); **Nama belakang ganda** menggabungkan dua nama belakang dengan tanda hubung. **Nama depan diawali** / **Nama belakang diawali** membatasi nama hanya pada yang diawali huruf-huruf tersebut.
4. Klik **Buat Nama**. Pilih nama lalu klik **Salin** (semua nama jika tidak ada yang dipilih) atau **Sisipkan ke Teks** (bisa juga dengan klik ganda) untuk menaruhnya di kursor editor.

Perempuan mendapat bentuk nama belakang untuk perempuan jika bahasanya memilikinya (Polandia *Kowalska*, Ceko *Nováková*, Rusia *Иванова*). Nama Jepang, Tiongkok, Korea, Hungaria, dan Vietnam menampilkan nama keluarga lebih dulu. Beberapa daftar menggunakan aksaranya sendiri (Rusia, Yunani, Ibrani, Arab, dan lainnya); nama daftar tersebut menyatakannya.

**Tambah Daftar Sendiri…** mengubah berkas teks dengan satu nama per baris menjadi daftar milik Anda sendiri (nama depan, nama depan perempuan atau laki-laki, atau nama belakang). Pilih daftar lalu klik **Hapus Daftar Sendiri** untuk menghapusnya. Daftar nama berasal dari proyek sumber terbuka *Faker* (lisensi MIT); daftar itu berisi nama yang sedang digunakan, bukan daftar nama bersejarah yang telah diperiksa.

### Catatan Sementara

Catatan sementara menyimpan catatan di luar semua proyek: gagasan saat proyek lain sedang terbuka, kutipan yang disalin dari halaman web, gambar dari sebagian layar. Buka dengan **Jendela > Catatan Sementara** (**Alt+Shift+Return**).

- **+** memulai catatan baru (panah di sampingnya: teks berformat atau teks biasa). Judul di atas teks adalah nama catatan. **−** menghapus catatan yang dipilih.
- Catatan berformat mendukung **tebal**, *miring*, dan garis bawah (tombol atau **Ctrl+B / I / U**) serta gambar: tempel gambar, atau klik **kamera** untuk mengambil gambar sebagian layar. Kamera hanya tersedia untuk catatan berformat.
- Semuanya disimpan secara otomatis saat Anda mengetik. Jika sebuah catatan tidak dapat disimpan (disk penuh, folder catatan yang hilang), baris status memberi tahu dan catatan tetap terbuka beserta teksnya: catatan lain, catatan baru, atau folder catatan lain baru terbuka setelah catatan itu tersimpan, dan sebuah jendela menawarkan **Salin Teks** dan **Coba Lagi**. Jika program berakhir saat sebuah catatan tidak dapat disimpan, teksnya masuk ke folder darurat *~/.local/share/{{APP}}/Emergency* (0.65).
- **Kirim ke Proyek** mencantumkan setiap proyek yang terbuka, termasuk yang terbuka di jendela {{APP}} lain. Untuk masing-masing: **Dokumen Baru di Folder Riset**, **Dokumen Baru di** folder pilihan Anda, atau **Tambahkan ke Akhir** dokumen teks. Pemformatan dan gambar ikut terbawa. Catatan tetap berada di catatan sementara sampai Anda menghapusnya.

Catatan-catatan itu adalah berkas biasa dalam sebuah folder, biasanya *Documents/{{APP}} Scratch Pad* (teks biasa sebagai .txt, catatan berformat sebagai .html dengan gambarnya di subfolder tersembunyi *.images*). Pilih folder lain, jenis catatan baru, dan apakah catatan sementara tetap berada di atas jendela lain di **Berkas > Opsi > Umum > Catatan Sementara**. Catatan yang diubah di tempat lain (jendela {{APP}} lain, folder yang disinkronkan) langsung muncul.

**Dari program mana pun.** Pintasan di dalam program bekerja selama {{APP}} menerima masukan papan ketik. Untuk membuka catatan sementara dari program mana pun, biarkan desktop menjalankan {{APP}} dengan `--scratch-pad`: di KDE, **System Settings > Keyboard > Shortcuts > Add New > Command or Script**, masukkan perintah yang ditampilkan di **Berkas > Opsi > Papan Ketik** (tombol **Salin Perintah**), lalu tekan tombol yang Anda inginkan. Catatan sementara kemudian terbuka sendiri, tanpa jendela proyek, dan mengirim catatan ke proyek mana pun yang sedang terbuka. Jika {{APP}} telah ditambahkan ke menu aplikasi (**Bantuan > Tambahkan ke Menu Aplikasi**), entri menu itu juga memiliki tindakan **Catatan Sementara**.

**Kamera** mengambil gambar dengan program tangkapan layar KDE, *Spectacle*, bila terpasang (diperlukan di Wayland, tempat program tidak boleh membaca layar sendiri): seret pada bagian yang Anda inginkan. Tanpa Spectacle, di X11, {{APP}} menampilkan layar dan Anda menyeret pada bagian yang diinginkan; **Esc** membatalkan, **Enter** mengambil seluruh layar.

### Transkripsi Suara

Berbicaralah, bukan mengetik: pengaya **Transkripsi Suara** (versi 0.61 atau lebih baru) menuliskan apa yang Anda ucapkan. Semuanya berlangsung di komputer ini; tidak ada yang dikirim ke mana pun selama Anda berbicara.

**Versi 1.10 (dengan {{APP}} 0.99.0)** melakukan pengenalannya di **Mesin AI** yang dipakai bersama oleh fungsi-fungsi AI {{APP}}. Pengaya ini memerlukan {{APP}} 0.99.0 atau yang lebih baru (program yang lebih lama tetap memakai versi 1.9, yang memiliki whisper.cpp sendiri); halaman opsinya dimulai dengan panel yang sama seperti Baca Nyaring dan Tulisan Tangan, tempat Anda memasang mesin sekali saja dan, nanti, model dari daftar program. Unduhan model dari pengaya sendiri (di bawah) tetap berfungsi. Jumlah inti dan waktu setelah model yang tidak dipakai dibebaskan tidak lagi diatur di sini, melainkan untuk semua fungsi AI bersama-sama di **Berkas > Opsi > Umum > Model AI**.

**Sebelum rekaman pertama: pilih model.** Pengenalan suara memerlukan model, yang diunduh satu kali. Buka **Berkas > Opsi > Ucapan** (atau panah di samping tombol mikrofon, **Opsi Transkripsi Suara…**), pilih model, lalu tekan **Unduh dan Gunakan**:

- **Small**: unduhan 190 MB, cepat; cocok untuk ucapan yang jelas, terjemahan hanya kasar.
- **Medium** (disarankan): 539 MB; mampu mengimbangi ucapan pada sebagian besar prosesor beberapa tahun terakhir; pengenalan dan terjemahan baik.
- **Large v3**: sekitar 1,1 GB; yang terbaik, tetapi pada prosesor sering lebih lambat daripada ucapan, sehingga pada ucapan panjang teks tertinggal. Kartu grafis membantu.
- **Large v3 Turbo**: 574 MB; mendekati Large v3 dan lebih cepat, tetapi tidak dapat menerjemahkan.

Model berasal dari berkas model whisper.cpp di Hugging Face. Di bawah **Sumber** Anda dapat memberikan alamat lain, atau memakai **Pakai Berkas yang Saya Unduh…** untuk berkas model yang Anda ambil sendiri. Sebelum dipakai, model diperiksa: pertama checksum-nya (SHA-256), lalu deskripsi model di dalam berkas, dan transkripsi uji singkat. Hanya bila semuanya baik, model itu menggantikan model yang sedang dipakai, yang kemudian dihapus. Model dari sumber resmi harus memiliki checksum yang dikenal; jika tidak, model tidak dipasang (unduhan mungkin rusak). Berkas dari alamat lain, atau yang Anda unduh sendiri, yang checksum-nya bukan yang dikenal, hanya dibaca setelah Anda menyetujuinya, karena mesin ucapan harus membacanya. Alamat lain harus diawali *https://*. Unduhan yang terputus dilanjutkan dari titik berhentinya pada kesempatan berikutnya. **Uji Kecepatan** menunjukkan berapa lama komputer ini memproses rekaman singkat dengan model dan perangkat yang dipilih.

**Merekam.** Tekan tombol mikrofon di bilah alat Alat (atau **Edit > Alat Menulis > Transkripsi Suara**). Inspektor terbuka pada tab **Transkripsi** (mikrofon, yang pertama di deretan tombol inspektor), dan transkripsi baru dimulai, dengan nama berupa tanggal dan waktu. Berbicaralah; jeda singkat mengakhiri satu potongan, dan teksnya muncul beberapa detik kemudian. Jeda yang lebih panjang (3 detik kecuali Anda mengubahnya) memulai paragraf baru. Tekan tombol itu lagi untuk berhenti: potongan yang masih menunggu ditulis, lalu teks dapat diedit. Panah di samping tombol memilih mikrofon.

**Transkripsi** adalah berkas teks biasa dalam folder tersendiri, di luar semua proyek (biasanya *Documents/{{APP}} Transcriptions*), sehingga proyek Anda tetap seperti semula. Di tab ini Anda dapat mengedit transkripsi (disimpan otomatis), mengganti namanya, menyalin seluruh teks untuk ditempel di mana pun Anda perlukan, memasukkannya ke teks Anda dengan **Sisipkan pada Kursor** (bagian transkripsi yang ditandai, atau seluruhnya bila tidak ada yang ditandai, masuk ke kursor teks yang sedang Anda edit, sebagai satu langkah untuk Urungkan), menghapusnya (masuk ke tempat sampah desktop), atau membuka foldernya. Selama rekaman sedang ditulis, teksnya hanya-baca. Jika transkripsi yang diedit tidak dapat disimpan, transkripsi itu tetap terbuka (baris status menyebutkan alasannya); bila rekaman dimulai atau program berakhir pada saat itu, teksnya masuk ke folder darurat *~/.local/share/{{APP}}/Emergency* (0.65).

**Rekam sekarang, tulis teksnya nanti** (0.64). Menulis teks memerlukan banyak daya komputasi; pada laptop yang memakai baterai (atau ponsel) Anda dapat hanya merekam dan membiarkan teksnya ditulis kemudian. Pilih **Hanya rekam** di **Berkas > Opsi > Ucapan** (*Saat mikrofon dimulai*; di Android ini adalah standarnya). Tombol mikrofon kemudian hanya merekam: tidak ada hal lain yang berjalan, tidak ada model yang dimuat. Rekaman disimpan sebagai berkas suara (WAV, sekitar 1,9 MB per menit) di samping transkripsi, dengan jenis nama yang sama, dan muncul di daftar tab dengan tombol **Transkripsikan** pada barisnya; **Transkripsikan Semua** menulis teks setiap rekaman yang belum memilikinya, satu per satu. Teks kemudian muncul seperti pada rekaman langsung dan dapat diedit. Rekaman tetap disimpan setelah teksnya ditulis kecuali Anda mencentang **Hapus rekaman setelah teksnya ditulis**; mengganti nama atau menghapus transkripsi turut membawa rekamannya. Untuk menulis teks di komputer lain, pilih folder transkripsi yang dijaga tetap sama di kedua komputer oleh program sinkronisasi Anda. Jika rekaman dihentikan saat teksnya sedang ditulis (atau program berakhir), tombol **Transkripsikan** tetap ada dan prosesnya dapat dijalankan lagi.

**Bahasa.** **Bahasa utama** adalah bahasa teks; **bahasa cadangan** adalah bahasa lain yang juga Anda gunakan (keduanya dipilih dari sebelas bahasa program: Inggris, Jerman, Spanyol, Prancis, Portugis, Italia, Rusia, Indonesia, Turki, Polandia, dan Belanda). Dengan bahasa Inggris sebagai bahasa utama, apa yang Anda ucapkan dalam bahasa cadangan ditulis dalam bahasa Inggris (whisper hanya menerjemahkan ke bahasa Inggris). Kata dari bahasa cadangan di tengah kalimat bahasa Inggris dapat salah dengar (dalam sebuah uji, "Bahnhof" menjadi "Bon Huff"); taruh kata yang Anda gunakan di bawah **Kata petunjuk**, dan kata itu ditulis seperti yang Anda ucapkan ("Bahnhof", "Brötchen"). Dengan bahasa utama lain, atau dengan Large v3 Turbo, ucapan tetap dalam bahasa saat diucapkan. **Simpan ucapan asli di samping terjemahan** menambahkan teks asli dalam tanda kurung setelah potongan yang diterjemahkan (memerlukan waktu lebih lama). **Kata petunjuk**: nama dan kata yang Anda gunakan, agar dieja menurut cara Anda.

**Opsi lain.** **Hitung di**: **Otomatis** (kartu grafis yang memori kosongnya memuat model, jika tidak prosesor), prosesor, atau salah satu kartu grafis yang ditemukan mesin AI (NVIDIA, AMD, dan Intel melalui Vulkan; daftarnya menampilkannya setelah mesin terpasang). Kolom versi 1.9 untuk thread prosesor dan waktu untuk membebaskan model sudah tidak ada: keduanya adalah satu pengaturan program di **Berkas > Opsi > Umum > Model AI**. **Akurasi**: *Cermat* menimbang beberapa kemungkinan teks (beam search) dan lebih dulu menentukan bahasa setiap potongan, di antara kedua bahasa Anda: lebih baik, tetapi lebih lambat (ini memerlukan paket mesin AI dari {{APP}} 0.99.0 atau yang lebih baru; paket mesin yang lebih lama mengabaikannya). **Tulis ke dalam teks saat saya berbicara** (mati pada awalnya): setiap potongan ucapan langsung juga disisipkan pada kursor teks yang sedang Anda edit; transkripsi tetap disimpan di tab seperti sebelumnya. **Mikrofon**, **Sensitivitas** (rendah untuk ruangan bising, tinggi untuk suara yang pelan), **Jeda yang mengakhiri satu potongan**, **Paragraf baru setelah jeda selama** (*tidak pernah* mematikannya), **folder**, dan di bawah **Lanjutan**: mengabaikan teks pada saat hening dan bising (whisper terkadang mengarang kalimat seperti "Thank you for watching" di sana), dan memakai teks sebelumnya sebagai konteks untuk potongan berikutnya.

---

### Asisten Penerjemah

Sejak 0.92.0, pengaya **Asisten Penerjemah** (versi 1.6; namanya sampai 1.4 adalah Terjemahan dan Pemeriksaan Tulisan) menjadi salah satu pengaya bawaan {{APP}} dan disertakan dalam versi lengkap; untuk {{APP}} yang sudah Anda miliki, pengaya ini berupa paket tersendiri di halaman Releases (bagian 17). Pengaya ini menerjemahkan antara dua bahasa mana pun dari sebelas bahasa dan memeriksa ejaan dan tata bahasa teks berbahasa Inggris, di komputer Anda sendiri: teks tidak pernah meninggalkannya. {{APP}} sendiri tidak memiliki fungsi AI; pengaya ini bekerja dengan model bahasa dan dapat dimatikan atau dihapus di Pengelola Pengaya seperti pengaya lainnya.

- **Alat ini hanya memberi saran. Terjemahannya tidak tepat: periksa setiap terjemahan, yang memerlukan pengetahuan kerja tentang kedua bahasa.** Kalimat yang sama tercantum di jendela pengaya dan di deskripsinya.
- Pengaya ini memiliki tab tersendiri di inspektor dan jendela tersendiri (perintahnya ada di submenu Alat Menulis pada menu Edit, dan ada tombol bilah alatnya). Teks dimasukkan, hasil keluar dengan setiap perubahan ditandai, langsung saat Anda mengetik; klik pada sebuah kata untuk mempertahankan aslinya atau untuk melihat kata lain.
- Pengaya ini memerlukan model, yang Anda pilih dan unduh satu kali di opsi pengaya (beberapa ratus megabita sampai beberapa gigabita; model yang terlalu besar untuk komputer tidak ditawarkan). Model diambil dari rilis *models-1*, *models-2*, *models-3*, dan *models-4* di repositori {{APP}}, masing-masing dipasang hanya bila checksum-nya adalah yang dikenal, dan sesudah itu pengaya bekerja tanpa koneksi apa pun. Satu model, CoEdIT Large, hanya boleh digunakan untuk tujuan nonkomersial; pengaya menyatakannya sebelum model diunduh.
- **Satu kata saja** (1.3). Satu kata bahasa Inggris: hasilnya adalah kata sebagaimana seharusnya, dan di bawahnya artinya (arti berasal dari kamus WordNet 3.0, yang tertanam di dalam pengaya; kata yang tidak ada di dalamnya mendapat arti yang ditulis oleh model Qwen3 yang telah Anda unduh, dengan peringatan bahwa mungkin tidak akurat, dan tanpa model semacam itu pengaya menyebutkan apa yang dapat Anda unduh), saran ejaannya, dan kata yang bermakna mirip. Satu kata bahasa Jerman: pengaya mencarinya lebih dulu di **kamus bahasa Jerman**, yaitu kutipan dari Wiktionary yang Anda unduh terpisah di opsi pengaya (lisensinya, CC BY-SA 4.0, ditampilkan sebelum unduhan): bentuk dasar, kelas kata, arti, dan kata bahasa Inggris untuk setiap arti. Lima arti pertama dari satu kelas ditampilkan, sisanya berada di balik "more"; klik pada kata bahasa Inggris menjadikannya hasil, dan "Similar" kemudian mengikuti kata itu beserta kelas katanya. Untuk hal yang tidak disebutkan kamus, dan untuk kata yang tidak ada di dalamnya, model kecil menerjemahkan kata tersebut, dan keluarannya memperingatkan bahwa hasilnya mungkin tidak akurat. Bahasa Inggris ke bahasa Jerman bekerja dengan cara yang sama untuk satu kata bahasa Inggris, menurut kelas kata yang dikenal WordNet.
- **Tukar dan Terjemahkan balik** (1.3). **Tukar** mengubah Jerman ke Inggris menjadi Inggris ke Jerman dan sebaliknya: hasilnya berpindah ke dalam teks dan diterjemahkan ke arah sebaliknya. **Terjemahkan balik** menerjemahkan hasil kembali ke bahasa teks Anda dan menampilkannya di bawah hasil, sampai Anda mengubah teksnya; ini memerlukan model untuk arah sebaliknya (OPUS-MT English to German (small) untuk teks bahasa Jerman), dan menawarkan untuk mengunduhnya bila belum ada. Selama Anda menerjemahkan, baris kedua kontrol menampilkan model terjemahan (memilih yang lain mengubah pengaturan dan terjemahannya), dan daftar gaya disembunyikan.
- **Teks pendek** (1.3). Satu kata dan teks hingga tiga kata tidak pernah diberikan kepada model terjemahan besar (model itu lebih baik untuk kalimat utuh dan tidak andal untuk kata tunggal): kamus lebih dulu, jika tidak model kecil, juga ketika model besar yang dipilih. Untuk teks pendek yang terdiri dari beberapa kata, "Other translations:" mencantumkan terjemahan lengkap terbaik dari model, tanpa nama yang dikarangnya sendiri.
- **Versi 1.4.** Satu kata ke kedua arah kini menampilkan, di bawah hasil, terjemahan untuk setiap kelas kata yang dimilikinya (kata bahasa Jerman yang berupa kata kerja dalam huruf kecil dan kata benda dengan huruf kapital mendapat keduanya; kata yang tidak ada di kamus diterjemahkan dalam kedua bentuk), lalu "Similar translations" (kata yang bermakna mirip dengan hasil bahasa Inggris) atau, dari bahasa Inggris ke bahasa Jerman, "Similar German words": kata bahasa Jerman yang artinya menyebut kata bahasa Inggris itu atau salah satu sinonimnya (ini memerlukan kamus bahasa Jerman; tanpa kamus satu baris menyatakannya), lalu deskripsi singkat. Dengan "Automatic", satu kata bahasa Jerman yang bukan kata bahasa Inggris dikenali sebagai bahasa Jerman bila kamus bahasa Jerman terpasang. Sebelum bahasa Jerman diterjemahkan, huruf kapital kata benda bahasa Jerman dibetulkan pada salinan yang dikirim ke model (teks Anda tetap seperti semula): kata yang hanya dikenal kamus sebagai kata benda mendapat huruf kapitalnya, kata yang dikenal dalam kedua bentuk ("essen") mempertahankan bentuk yang Anda ketik, kecuali tepat setelah artikel atau preposisi dengan artikel ("das Essen", "im Essen"); tanpa kamus, catatan di bawah terjemahan menyatakan bahwa kata benda bahasa Jerman memerlukan huruf kapital. Alternatif yang berbeda satu sama lain atau dari terjemahan hanya pada huruf kapital ditampilkan sekali saja. Area kata tidak lagi menampilkan baris "Spelling" untuk kata hasil (kata yang Anda klik di teks Anda sendiri tetap memilikinya), dan aturan akhiran untuk kata-kata serupa tidak lagi membaca "number" sebagai "numb". Bila ada yang hilang (model terjemahan, kamus bahasa Jerman), pemberitahuan di bagian atas tab dan jendela pengaya menyebutkannya dan memiliki tombol yang membuka opsi pengaya. Model yang tidak dipakai selama 15 menit dibongkar dari memori untuk membebaskan memori (**Berkas > Opsi > Pemeriksaan Tulisan > Kecepatan > Bongkar model setelah**; "tidak pernah" membuatnya tetap dimuat). Petunjuk di kolom kosong dibungkus pada lebar yang sempit.
- **Versi 1.5: sebelas bahasa.** Pengaya menerjemahkan antara dua bahasa mana pun dari Inggris, Jerman, Spanyol, Prancis, Portugis (Brasil), Italia, Rusia, Indonesia, Turki, Polandia, dan Belanda: dengan **Terjemahkan** dipilih, dua daftar tarik-turun menyebutkan bahasa teks Anda dan bahasa terjemahannya. Setiap bahasa memiliki dua model OPUS-MT kecil, ke dan dari bahasa Inggris (lisensinya, Apache-2.0 atau CC BY 4.0, ditampilkan di opsi sebelum unduhan); pasangan tanpa bahasa Inggris (misalnya Spanyol ke Rusia) melewati bahasa Inggris dengan kedua model, dan catatan di bawah hasil menyatakannya, karena kesalahan pada langkah pertama terbawa ke langkah kedua. Setiap bahasa selain Inggris juga memiliki **kamus** opsional dari Wiktionary (CC BY-SA 4.0, unduhan terpisah yang lisensinya ditampilkan lebih dulu), yang dipakai untuk satu kata bahasa itu seperti kamus bahasa Jerman dahulu. Bahasa tanpa kamus yang dapat dipakai hanya mendapat terjemahan model. Satu kata tetap dikoreksi sebagai saran: baris ejaan dan kata di bawahnya menyatakan bahwa semuanya saran yang harus diperiksa. Teks pengaya sendiri tersedia dalam sebelas bahasa program (bahasa yang Anda pilih untuk {{APP}}; bahasa Inggris bila suatu teks belum diterjemahkan). Pengaturan 1.4 tetap berfungsi: kamus bahasa Jerman dan model yang Anda unduh tetap ada, dan Jerman ke Inggris adalah pilihan pertama. Opsinya kini ada di **Berkas > Opsi > Asisten Penerjemah**.
- **Versi 1.6: satu aturan untuk memori.** Dengan {{APP}} 0.99.0 atau yang lebih baru, pengaya melaporkan modelnya ke daftar model program: sebelum sebuah model dimuat, program memeriksa bahwa model itu muat di memori (dan dapat lebih dulu mengeluarkan model yang paling lama tidak dipakai, atau menolak pemuatan dan menyebutkan alasannya di baris pesan); model yang tidak dipakai selama waktu yang ditetapkan dikeluarkan oleh program; selagi sebuah model dimuat atau dikeluarkan, baris pesan menyatakan "Perubahan model sedang berlangsung". Jumlah inti prosesor dan waktu setelah model dikeluarkan tidak lagi diatur di opsi pengaya: keduanya adalah pengaturan program untuk semua fungsi AI, **Berkas > Opsi > Umum > Model AI**, dan halaman opsi Asisten Penerjemah menyebutkannya. Dengan {{APP}} yang lebih lama, pengaya tetap memakai kedua kolomnya sendiri. Pustaka mesin CTranslate2 adalah versi 4.9.1 (Linux).
- Model yang Anda unduh dengan versi 1.1 tetap seperti semula. Versi 1.1 mengambil model dari repositori pengaya yang lama; 1.2 mengenal tempat yang baru.
- Deskripsi pengaya itu sendiri, beserta aturannya untuk pemeriksaan tulisan (ejaan Amerika, tanda kutip melengkung, tanpa em dash) dan keunggulan setiap model, adalah *addons/translation-proofreading/source/README.md* di sumber {{APP}}.

## 11. Snapshot

Snapshot adalah salinan tersimpan dari sebuah dokumen pada saat tertentu. Buatlah sebelum penulisan ulang besar-besaran:

- Dokumen dengan setidaknya satu snapshot menampilkan sudut terlipat (terang dan abu-abu) di kanan atas ikonnya di binder.
- **Ctrl+5** membuat snapshot; **Ctrl+Shift+5** menanyakan judul.
- Dengan **Berkas > Opsi > Umum > Menyimpan**, **Berkas > Simpan** (**Ctrl+S**) membuat snapshot dari setiap teks yang Anda ubah sejak Berkas > Simpan terakhir (berjudul "Saved"); **Berkas > Opsi > Perilaku > Snapshot** menampilkan pesan singkat di bilah status saat snapshot dibuat.
- Di tab snapshot inspektor Anda dapat **membandingkan** snapshot dengan teks saat ini (penambahan ditampilkan dengan warna, penghapusan dicoret; pilih apakah membandingkan kata, kalimat, paragraf, atau karakter), dan **memulihkan** ke snapshot itu. Sebelum memulihkan, {{APP}} dapat membuat snapshot dari teks saat ini, sehingga tidak ada yang hilang. Snapshot dibaca lebih dulu (0.65): snapshot yang tidak dapat dibaca utuh membiarkan teks saat ini apa adanya, snapshot yang tidak berisi teks menggantikannya hanya setelah pertanyaan kedua, dan bila snapshot dari teks saat ini diminta tetapi tidak dapat dibuat, tidak ada yang dipulihkan. Memulihkan juga merupakan cara memperbaiki dokumen yang berkasnya rusak atau berada dalam pengodean karakter lain (ditampilkan hanya-baca): berkas yang rusak lebih dulu disimpan sebagai salinan di *settings/conflicts*, dan inspektor menyebutkan lokasinya (0.81.1).
- **Proyek > Hapus Snapshot Lama…** (sejak 0.61.1, di bawah Kosongkan Tempat Sampah) menghapus setiap snapshot proyek, termasuk milik dokumen di Tempat Sampah, agar proyek menjadi lebih kecil lagi. Teks tetap seperti sekarang; hanya versi-versi sebelumnya yang hilang. Pertanyaan lebih dulu menyebutkan berapa banyak snapshot yang ada, dari berapa banyak dokumen, dan berapa ruang yang dipakainya; ini tidak dapat dibatalkan, jadi buatlah cadangan lebih dulu (**Berkas > Cadangkan**). Perintah ini mati selama proyek tidak memiliki snapshot. Pada proyek berformat Scrivener, snapshot juga keluar dari *.scriv* (dengan penyimpanan yang langsung menyusul); sejak 0.61.1 hal ini berlaku juga untuk satu snapshot yang Anda hapus di inspektor.

---

## 12. Mode menulis

**F11** memenuhi layar dengan hanya teks Anda di atas selembar kertas. Gerakkan tetikus ke tepi bawah untuk menampilkan bilah kontrol: zoom, lebar kertas, pemudaran latar, pengguliran mesin ketik, jumlah kata, dan inspektor kecil dengan sinopsis dan catatan. **Alt+Shift+Up / Down** berpindah ke dokumen sebelumnya atau berikutnya, **Ctrl+[** dan **Ctrl+]** mundur dan maju melalui dokumen yang telah Anda lihat. Pintasan menu Format dan Sisipkan (tebal, gaya, komentar ...) juga bekerja di sini. **Esc** atau **F11** kembali ke jendela utama.

**Berkas > Opsi > Perilaku > Mode Menulis** memilih layar tempat mode ini terbuka (layar saat ini, layar utama, atau layar menurut nama), apakah jendela utama menyingkir (diminimalkan selama mode menulis terbuka), dan apakah **Esc** menutupnya (**F11** selalu menutupnya).

Bilah kontrol berwarna gelap di setiap tema, sehingga tombol, daftar zoom, dan teksnya tetap terbaca di tema terang juga (0.94.0). Warna teks, kertas, dan latar berasal dari tema. Untuk mengubahnya, misalnya menjadi hitam di atas putih atau hijau di atas hitam, buka **Berkas > Opsi > Penampilan > Mode Menulis**; perubahan langsung terlihat, bahkan saat mode menulis sedang terbuka. Gambar latar dapat ditetapkan per proyek di bawah **Pengaturan Proyek > Folder Khusus**.

---

## 13. Kompilasi: menyusun naskah

**Berkas > Kompilasikan…** (**Ctrl+Shift+E**) menyatukan seluruh isi Draf menjadi satu berkas.

### Jendela kompilasi

Jendela kompilasi memiliki **Kompilasi Untuk** di bagian atas, format di sebelah kiri, tata letak bagian di tengah, serta konten, metadata, opsi, dan penggantian di sebelah kanan (empat tab dengan ikon; arahkan tetikus ke sebuah tab untuk melihat namanya). Kolom Format selebar nama terpanjangnya, paling tidak; seret garis di antara ketiga kolom untuk melebarkan salah satunya, dan jendela mengingat lebarnya untuk lain kali.

1. **Kompilasi Untuk**: format keluaran. Bawaan, tanpa memerlukan program lain: **Dokumen Word (.docx)**, **PDF (.pdf)**, **E-book (.epub)**, dan untuk skenario **Fountain** dan **Final Draft**. PDF dan OpenDocument melalui LibreOffice, EPUB melalui Pandoc, dan Markdown juga muncul bila plugin dan program yang sesuai terpasang (lihat di bawah). Di sampingnya: berapa banyak dokumen dan kira-kira berapa banyak kata yang akan dikompilasi.
2. **Format** (kiri), di bawah **Format Saya** dan **Format Bawaan**: format kompilasi menentukan tampilan naskah: font, jarak baris, ukuran halaman, margin, header dan footer, tampilan bab dan adegan, serta bagaimana gaya, catatan, dan kutipan dihasilkan. Format bawaan (*Manuscript (Times)*, *Manuscript (Courier)*, *Modern*, *Paperback (6 x 9 in)*, *E-book*, *Default (Editor Formatting)*, dan *Outline (Synopses)*) tidak dapat diubah. Tombol di bawah daftar: **+** (menggandakan format yang dipilih untuk membuat versi Anda sendiri dan mengeditnya), pensil (mengedit format Anda sendiri), **-** (menghapusnya), **Impor...** dan **Ekspor...** (berkas *.qbformat* untuk dipakai di komputer lain atau diberikan kepada orang lain; format yang diimpor dengan nama yang sudah dipakai diberi nomor).
3. **Tata Letak Bagian** (tengah): **Font** menetapkan satu font untuk seluruh naskah, atau menyerahkannya kepada tata letak ("Ditentukan oleh Tata Letak Bagian"). Tabel mencantumkan setiap jenis bagian di proyek Anda beserta tata letak yang didapatnya dalam format ini; pilih yang lain di daftar. Di bawah tabel, gambar kecil menunjukkan bagaimana sebuah bagian dengan tata letak terpilih dimulai. **Tetapkan Tata Letak Bagian...** membuka jendela yang lebih besar dengan jenis bagian di sebelah kiri dan gambar semua tata letak format di sebelah kanan. Format bawaan mengenal nama umum jenis bagian: di *Manuscript (Courier)*, misalnya, *Part* mendapat **Part Title**, *Chapter* mendapat **Chapter Heading**, dan *Scene* atau *Section* mendapat **Section Text**.
4. **Konten** (tab pertama): apa yang dikompilasi, beserta jenis bagian dan tata letak setiap dokumen. Pilih folder untuk mengompilasi hanya sebagian Draf (misalnya satu bab), tambahkan bagian depan (halaman judul) dan bagian belakang (ucapan terima kasih). **Nama penulis** (0.94.0): halaman judul pada templat proyek berbunyi "oleh" dan nama penulis. Kompilasi pertama yang menulis halaman judul menanyakan nama tersebut (nama depan dan belakang; disimpan bersama Informasi Penulis di **Berkas > Opsi > Umum**, sehingga hanya ditanyakan sekali); jika Anda mengosongkannya, baris "oleh" dihilangkan dari halaman judul, di setiap format. Kolom Penulis pada tab Metadata jendela kompilasi, bila berisi nama, dipakai untuk berkas hasil kompilasi sebagai gantinya. Hapus centang pada dokumen untuk mengecualikannya; ini mematikan pengaturan **Sertakan dalam Kompilasi**-nya. **Tambahkan lembar objek** menaruh lembar untuk setiap objek pada tab Objek (bagian 7) setelah bagian belakang: judul per kategori (Tokoh, Lokasi ...), lalu setiap objek beserta kolomnya; tombol di sampingnya memilih kategorinya. Pada awalnya dalam keadaan mati.
5. **Metadata** (tab kedua): judul, subjudul, penulis, bahasa (misalnya *en-US*), subjek, kata kunci, penerbit, tanggal, deskripsi, sampul e-book (gambar dari binder), dan pengenal buku. Kolom kosong memakai judul proyek, nama Anda, dan tanggal hari ini. Rinciannya masuk ke properti dokumen PDF, e-book, dan berkas Word; placeholder seperti `<$projecttitle>` juga berfungsi di sini.
6. **Opsi** (tab ketiga): apakah teks folder dikompilasi.
7. **Penggantian** (tab keempat): penggantian untuk proyek ini, dilakukan setelah penggantian milik format, misalnya nama yang dieja satu cara di draf Anda dan cara lain di buku. Setiap baris dapat dinyalakan dan dimatikan, dan dapat berupa ekspresi reguler, peka huruf besar-kecil, atau hanya kata utuh.
8. **Pratinjau** menampilkan halaman naskah sebagai PDF di jendela tersendiri, dengan format dan konten yang Anda pilih, sebelum apa pun disimpan.

Font, penggantian, dan lembar objek disimpan bersama proyek seperti pilihan lainnya (versi {{APP}} yang lebih lama mengabaikannya).

Klik **Kompilasikan…**, pilih lokasi penyimpanan, dan berkas yang sudah jadi terbuka di pengolah kata atau pembaca Anda jika Anda mau. Sebuah jendela menampilkan kemajuan ("12 dari 148": jumlah dokumen yang selesai dan totalnya) dengan tombol **Batal**; **Esc** melakukan hal yang sama. Batal berhenti setelah dokumen yang sedang dibuat: tidak ada berkas yang dibuat, berkas bernama sama dari sebelumnya tetap seperti semula (berkas baru ditulis dengan nama sementara di sampingnya dan dipindahkan ke tempatnya hanya bila sudah utuh), dan konverter seperti LibreOffice atau Pandoc dihentikan. Baris pesan di bawah binder menyatakan bahwa kompilasi dibatalkan. **Simpan Pengaturan** menyimpan pilihan tanpa mengompilasi.

**Font dan pemformatan paragraf yang ditetapkan secara manual.** Dengan format *Default (Editor Formatting)*, berkas hasil kompilasi mempertahankan semuanya: font, ukuran, warna, huruf kapital, spasi huruf, indentasi, spasi, jarak baris, dan perhentian tab. Setiap format lain memiliki tampilan sendiri: format itu menggantikan font, ukuran, indentasi, dan spasi, dan mempertahankan warna teks, huruf kapital, spasi huruf, perataan, dan perhentian tab.

### Mengedit format kompilasi

**Duplikat & Edit…** dan **Edit…** membuka editor format. Daftar di sebelah kiri berisi halamannya:

- **Teks**: font, ukuran, jarak baris, indentasi baris pertama, spasi setelah paragraf, teks rata kiri-kanan, font dan ukuran judul, judul tebal. **Pakai pemformatan editor proyek** mempertahankan tampilan editor.
- **Tata Letak Bagian**: tata letak format (tambah, duplikat, hapus) dengan judul, awalan dan akhiran (`Chapter <$W:chapter>`), gaya judul dan perataan, huruf besar, baris kosong sebelum judul, teks, sinopsis, dan catatan, halaman baru, pemisah sebelum bagian, tanpa indentasi pada paragraf pertama. Pratinjau menunjukkan bagaimana bagian dengan tata letak ini dimulai.
- **Penetapan**: tata letak mana yang didapat setiap jenis bagian dalam format ini.
- **Gaya**: bagaimana gaya proyek Anda dihasilkan dalam format ini. **Tambah gaya…**, lalu pilih: **Pertahankan tampilannya**, **Ubah tampilannya** (font, ukuran, tebal, miring, garis bawah, kapital kecil atau kapital penuh, warna, perataan, indentasi, spasi sebelum dan sesudah, jarak baris, teks sebelum dan sesudahnya, huruf besar, mulai di halaman baru; kotak yang tercentang setengah, atau "tidak berubah", membiarkan gaya apa adanya), **Jadikan teks isi**, atau **Abaikan teksnya** (misalnya catatan untuk diri sendiri dalam gaya tersendiri).
- **Halaman**: ukuran kertas dan margin, margin cermin untuk buku (dalam dan luar), header dan footer (kiri, tengah, kanan; placeholder seperti `<$p>` untuk nomor halaman dan `<$title>`), halaman berhadapan (teks kiri di halaman kiri, teks kanan di halaman kanan), header dan footer di halaman pertama dan di halaman tempat bab dimulai, bab di halaman kanan, nomor halaman pertama dan gayanya (1, 2, 3 / i, ii, iii / I, II, III), ukuran header dan footer.
- **Catatan Kaki dan Komentar**: catatan kaki di bagian bawah halaman (di e-book: catatan yang muncul sebagai jendela kecil), di akhir setiap bab, atau di akhir teks (PDF dan e-book; berkas Word selalu memiliki catatan kakinya di bagian bawah halaman), dengan judul catatan dan ukurannya; anotasi sebaris sebagai komentar atau dihapus; komentar tertaut sebagai komentar berkas Word; sorotan dihapus.
- **Penggantian**: teks yang diganti hanya di berkas hasil kompilasi (proyek Anda tetap seperti semula): hidup atau mati, huruf besar-kecil, kata utuh, ekspresi reguler (`\1` sampai `\9` pada "Dengan").
- **Transformasi**: tanda kutip lurus atau tipografis, tanda pisah sebagai tanda hubung, miring sebagai garis bawah (konvensi naskah), beberapa spasi menjadi satu, tanpa spasi di akhir paragraf, teks biasa tanpa tebal, miring, font, atau warna.
- **Daftar Isi, PDF dan E-book**: daftar isi (judulnya dan berapa tingkat judul), penanda PDF untuk judul, tautan web yang dapat diklik di PDF, satu berkas per bagian yang dimulai di halaman baru dalam e-book, dan daftar isi sebagai halaman e-book juga.

Format Anda disimpan di *~/.local/share/{{APP}}/CompileFormats* dan tersedia di setiap proyek.

### PDF

Keluaran PDF bawaan menata halaman sendiri: catatan kaki di bagian bawah halaman tempat catatan itu dirujuk (catatan yang panjang berlanjut ke halaman berikutnya), header dan footer, bab di halaman kanan jika format menyatakannya, judul dijaga tetap bersama teks yang mengikutinya, tidak ada baris tunggal di atas atau bawah halaman, daftar isi dengan nomor halaman yang dapat diklik, penanda untuk judul di panel samping pembaca, dan tautan yang dapat diklik. Font disematkan. Gambar mempertahankan ukurannya di dalam teks; gambar yang lebih besar dari area teks diperkecil agar muat. Gambar dan tabel di samping teks dikelilingi aliran teks, seperti di editor. Tabel berlanjut ke sebanyak mungkin halaman yang diperlukan, baris demi baris, dengan baris judulnya di bagian atas setiap halaman.

### E-book

Keluaran e-book bawaan menulis EPUB 3 (dengan daftar isi untuk pembaca yang lebih lama juga): satu berkas per bab, judul dalam daftar isi pembaca, catatan kaki sebagai catatan yang muncul sebagai jendela kecil (atau di akhir setiap bab atau di akhir buku), gambar, dan sampul dari **Metadata…**. Tabel mempertahankan desainnya; gambar dengan keterangan menjadi sebuah figur (di samping teks jika demikian di editor), dan deskripsinya adalah teks alt gambar, yang dibacakan pembaca layar. Buku mendapat pengenal saat pertama kali dikompilasi; pengenal itu tetap bersama proyek, sehingga pembaca menganggap versi baru sebagai buku yang sama. Pembaca memilih font dan ukuran halamannya sendiri, jadi pengaturan halaman, header, dan footer tidak berlaku.

### Word

Berkas Word memakai gaya bernama (Title, Subtitle, Heading 1 sampai 3, Quote, Centered, Caption, dan gaya paragraf dan karakter Anda sendiri), sehingga judulnya muncul di panel navigasi Word. Dengan **Sertakan daftar isi**, bidang daftar isi ditaruh di awal, dan berkas meminta pengolah kata memperbarui bidangnya saat dibuka sehingga nomor halaman terisi; jika pengolah kata tidak melakukannya, perbarui bidang itu sendiri di sana. Header berhadapan, halaman pertama, dan nomor halaman pertama berasal dari pengaturan **Halaman** format. Tabel adalah tabel Word dengan garis, warna, sel gabungan, dan baris judulnya (diulang di setiap halaman di Word). Gambar di samping teks dimasukkan ke tabel satu sel tanpa garis, bersama keterangannya, sehingga Word dan LibreOffice membiarkan teks mengalir di sekeliling keduanya; deskripsi gambar adalah teks alt-nya.

### Judul dan placeholder

Dalam tata letak bagian, judul dapat berisi placeholder yang diganti oleh Kompilasi:

| Placeholder | Hasil |
|-------------|--------|
| `<$n>` | 1, 2, 3 … |
| `<$w>` / `<$W>` / `<$WW>` | satu, dua … / Satu, Dua … / SATU, DUA … |
| `<$R>` / `<$r>` | I, II, III … / i, ii, iii … |
| `<$L>` / `<$l>` | A, B, C … / a, b, c … |
| `<$n:name>` | penghitung terpisah bernama *name* (berfungsi dengan setiap gaya nomor di atas) |
| `<$title>` | judul dokumen |
| `<$projecttitle>` / `<$shorttitle>` | judul proyek / tiga kata pertamanya dalam huruf kapital |
| `<$author>`, `<$forename>`, `<$surname>` | nama Anda dari **Berkas > Opsi** atau pengaturan proyek |
| `<$wc>` / `<$wc100>` / `<$wc1000>` | jumlah kata, tepat atau dibulatkan |
| `<$date>` / `<$year>` | tanggal hari ini (yyyy-mm-dd) / tahun |
| `<$p>` | nomor halaman (hanya header dan footer) |
| `<$pages>` | jumlah halaman (header dan footer berkas PDF dan cetakan) |

Contoh: judul `Chapter <$W:chapter>` menjadi *Chapter One*, *Chapter Two*, dan seterusnya.

Placeholder juga dapat diketik langsung ke dalam teks (**Sisipkan > Nomor Otomatis** dan **Sisipkan > Placeholder**). **Bantuan > Daftar Semua Placeholder** mencantumkan semuanya; pilih baris lalu tekan **Ctrl+C** untuk menyalinnya.

### Gaya saat kompilasi

Judul, Title, Subtitle, Block Quote, Centered, dan Caption mendapat tampilannya dari format kompilasi. Setiap gaya lain membawa pemformatannya sendiri (indentasi, spasi, perataan, font, warna), di setiap format kompilasi. Dengan **Default (Editor Formatting)**, perubahan yang Anda buat pada gaya bawaan juga ikut terbawa. Halaman **Gaya** pada sebuah format mengubah hal ini untuk setiap gaya (lihat *Mengedit format kompilasi*).

### Catatan saat kompilasi

Di halaman **Catatan Kaki dan Komentar** pada format kompilasi, Anda memilih ke mana catatan kaki pergi di PDF atau e-book (bagian bawah halaman, akhir setiap bab, akhir teks), apakah anotasi sebaris menjadi komentar di berkas Word atau dihapus, apakah komentar tertaut dipertahankan, dan apakah sorotan dihapus. Di berkas Word, catatan kaki selalu berupa catatan kaki sungguhan di bagian bawah halaman.

### Format keluaran lainnya (plugin)

{{APP}} juga hadir dengan format-format ini. Masing-masing memerlukan program yang dipasang terpisah; PDF dan EPUB melalui LibreOffice dan Pandoc adalah alternatif bagi yang bawaan (keduanya berangkat dari berkas Word dan tampilannya serupa):

| Format | Memerlukan |
|--------|-------|
| PDF (via LibreOffice) | LibreOffice (terpasang di sebagian besar sistem Ubuntu dan Kubuntu) |
| OpenDocument Text (.odt, via LibreOffice) | LibreOffice |
| E-book (.epub, via Pandoc) | Pandoc (pasang "pandoc" di Discover) |
| Markdown (.md) | tidak ada |

**Berkas > Opsi > Plugin** menunjukkan format mana yang siap dan mana yang kekurangan program. Format lain dapat dipasang sebagai pengaya (bagian 17).

### Mengompilasi skenario

Dua keluaran dibuat untuk skenario dan tidak memerlukan program lain:

- **Fountain Screenplay (.fountain)**: berkas teks biasa yang dibaca oleh Highland, Fade In, Beat, Trelby, dan program penulisan skenario lainnya.
- **Final Draft (.fdx)**: format berkas Final Draft, yang dapat dibuka juga oleh sebagian besar program penulisan skenario.

Pada keduanya, setiap paragraf mempertahankan jenisnya (judul adegan, aksi, tokoh, parentetikal, dialog, transisi, bidikan); program lain menata skenario dengan caranya sendiri. Judul folder dan dokumen yang ditambahkan format kompilasi menjadi bagian (Fountain) atau judul babak (Final Draft); teks yang bukan elemen skenario menjadi aksi. Judul proyek dan nama Anda (Berkas > Opsi) membentuk halaman judul. Anotasi sebaris menjadi catatan di berkas Fountain dan dihilangkan dari berkas Final Draft.

Untuk Word atau PDF, pilih format kompilasi **Default (Editor Formatting)**: format ini mempertahankan indentasi, huruf kapital, dan font elemen seperti yang Anda lihat di editor. Format kompilasi lainnya menggantikan font dan ukuran tetapi mempertahankan indentasi.

---

## 14. Impor dan ekspor

### Mengimpor berkas

**Berkas > Impor > Berkas…** (**Ctrl+Shift+I**) menambahkan berkas teks (.txt, .md, .rtf, .docx, .odt; berkas Word lama .doc dengan pengaya di bawah), skenario (.fountain, .fdx), gambar, PDF, audio dan video, serta berkas lain ke binder. Anda juga dapat menyeret berkas dari pengelola berkas ke binder. Berkas teks menjadi dokumen yang dapat diedit; gambar dan PDF ditampilkan di editor; audio dan video diputar di sana dengan pengaya Media Playback (bagian 6). Berkas yang tidak dapat disalin ke proyek (disk penuh, berkas yang tidak dapat dibaca) disebutkan dalam pesan dan tidak ditambahkan, sehingga tidak ada item kosong yang tertinggal (0.92.0).

Cara berkas dibaca diatur di **Berkas > Opsi > Berbagi > Impor**: apakah komentar berkas Word, LibreOffice, dan RTF menjadi anotasi sebaris (jika tidak, tetap menjadi komentar di samping teks), apakah catatan kaki menjadi catatan kaki sebaris (standarnya) atau catatan kaki tertaut, apakah gaya paragraf dan karakter berkas dihilangkan (teks mendapat tampilan proyek Anda; tebal, miring, catatan, dan tautan tetap), pengodean berkas teks biasa (otomatis: UTF-8, jika tidak Windows Western), dan apakah berkas *.html* menjadi dokumen teks, bukan dokumen halaman web.

Ke mana berkas pergi: pilih folder di binder untuk mengimpor ke dalamnya (berkas ditaruh paling akhir), atau dokumen untuk menaruhnya tepat di bawahnya. Draf hanya menerima teks; berkas lain yang dijatuhkan di sana masuk ke Riset.

**Impor dan Pisahkan…** membaca satu berkas panjang dan memotongnya menjadi banyak dokumen, misalnya di setiap baris yang hanya berisi `#`, atau di setiap dua baris kosong berturut-turut. Sejak 0.94.0 fungsi ini juga dapat memotong berkas .docx, .odt, atau .md **pada judulnya**: Heading 1 menjadi folder, Heading 2 menjadi folder atau dokumen (Anda memilih di jendela), teks di antara judul menjadi dokumen, dan judul yang lebih rendah tetap berada di dalam teks. Jendela menampilkan binder yang akan dihasilkan sebagai pohon sebelum Anda mengimpor. Judul dokumen yang tidak dinamai oleh judul mana pun mengikuti satu aturan, yang tertulis di jendela: baris pertama teksnya yang tidak kosong (dipersingkat menjadi 120 karakter bila lebih panjang), atau "<nama berkas> <nomor>" bila **Gunakan baris pertama setiap bagian sebagai judulnya** mati.

### Berkas Word lama (.doc)

Word 97 sampai 2003 menyimpan dokumen sebagai *.doc* (templat *.dot*). Berkas tersebut menjadi dokumen teks dengan pengaya **Word 97–2003 Import** (bagian 17); tanpanya, berkas .doc ditambahkan ke binder apa adanya, dan Anda diberi tahu pengaya mana yang membacanya. Pengaya ini tidak memerlukan program lain. Pengaya ini mempertahankan:

- teks, termasuk aksara Yunani, Sirilik, Asia, dan sejenisnya;
- judul (Heading 1 sampai 6), title, subtitle, kutipan, dan keterangan, yang mendapat gaya proyek Anda; gaya paragraf dokumen yang juga dimiliki proyek Anda (nama sama) mendapat gaya tersebut;
- tebal, miring, garis bawah, coret, kapital kecil dan kapital penuh, superskrip dan subskrip, sorotan; paragraf rata tengah, rata kanan, dan rata kiri-kanan;
- daftar berpoin dan bernomor beserta tingkatnya (nomor itu sendiri dibuat oleh {{APP}}, sehingga penomoran judul "Chapter 1" tidak ikut terbawa);
- tabel, sebagai satu baris per baris tabel dengan sel dipisahkan tab;
- catatan kaki dan catatan akhir (sebagai catatan kaki), komentar beserta kata yang dikomentari (nama penulis berada di awal komentar);
- tautan, hasil bidang (tanggal, nomor halaman seperti yang terakhir ditampilkan), pemisah halaman dan bagian, pemisah baris;
- gambar di dalam teks (JPEG, PNG, BMP, TIFF);
- pada proyek RTF juga font, ukuran, warna, dan indentasi yang ditetapkan secara manual (proyek Markdown mempertahankan tampilan proyek, seperti pada setiap impor).

Dihilangkan: header dan footer, gambar dan gambar gambar yang mengambang, gambar dalam format metafile lama (WMF, EMF), objek tersemat, teks tersembunyi, dan teks yang dihapus dengan pelacakan perubahan (teks yang disisipkan tetap ada). Teks kotak teks ditambahkan di akhir dokumen. Setelah impor, sebuah pesan menyebutkan apa yang dihilangkan.

**Berkas > Opsi > Berbagi > Konversi** dapat menjadikan LibreOffice pilihan pertama untuk berkas *.doc* (LibreOffice juga membaca header, gambar, dan berkas Word 6/95); bila cara yang dipilih gagal, cara lainnya dicoba. Berkas Word 6.0 dan Word 95, serta berkas yang tidak dapat dibaca pengaya, dikonversi dengan LibreOffice bila terpasang (di Ubuntu: `sudo apt install libreoffice-writer`); jika tidak, pesan menyatakannya. Berkas yang dilindungi kata sandi harus dibuka dan disimpan tanpa kata sandi lebih dulu. Berkas *.doc* yang sebenarnya berkas RTF, berkas Word 2007 (.docx), atau halaman web, seperti yang disimpan sebagian program, dibaca sebagai apa adanya, juga tanpa pengaya.

### Mengimpor halaman web

**Berkas > Impor > Halaman Web…** membawa halaman dari web ke proyek Anda. Pengaya **Web Page Import** (bagian 17) mengerjakannya; tanpanya, perintah ini memberi tahu Anda.

1. Pilih folder yang akan menerima halaman (jika tidak, halaman masuk ke **Riset**).
2. Pilih **Berkas > Impor > Halaman Web…**. Jika papan klip berisi alamat web, alamat itu sudah terisi; jika tidak, ketik atau tempel satu alamat.
3. Klik **Muat** (atau tekan Enter). Halaman dan gambarnya diambil, dan jendela menampilkan apa yang akan diimpor, dengan jumlah kata dan gambar, penulis, situs, dan tanggal.
4. Perbaiki **Judul** bila perlu, periksa pilihan di bawahnya, lalu klik **Impor**.

**Hanya artikel** mempertahankan teks yang Anda cari dan menghilangkan menu, iklan, pemberitahuan cookie, tombol berbagi, bagian komentar, dan daftar artikel lain. {{APP}} menemukan artikel dengan cara yang dipakai alat baca (teks dengan kalimat dan koma terbanyak serta tautan paling sedikit yang menang; nama seperti "article" atau "sidebar" pada halaman berpengaruh mendukung atau menolak sebuah bagian). Jika ada yang hilang, hapus centangnya untuk mengambil seluruh halaman. **Gambar** mengambil gambar artikel (yang sangat besar dibuat selebar 1600 piksel). Untuk halaman dari internet, gambar hanya diambil dari alamat publik: gambar yang berada di komputer Anda sendiri atau di jaringan pribadi (bahkan melalui pengalihan) dihilangkan, dan hasilnya menyebutkan berapa banyak. Halaman yang berasal dari komputer atau jaringan Anda sendiri diambil beserta semua gambarnya.

**Impor sebagai**:

- **Dokumen teks** (standar): judul, paragraf, kutipan, daftar, tabel (satu baris per baris), kode program, keterangan, tebal, miring, dan tautan menjadi dokumen biasa yang dapat Anda edit, anotasi, pisahkan, dan kompilasi.
- **Dokumen halaman web**: salinan artikel hanya-baca beserta gambarnya, untuk referensi. Tidak dihitung atau dikompilasi.
**Melihat dokumen halaman web** (0.81.3). Editor menampilkan halaman sebagai teks berformat sederhana, dengan alamatnya dan dua tombol di atasnya. **Buka Halaman Tersimpan di Browser** menampilkan halaman seperti saat disimpan, di browser web Anda: {{APP}} menaruh salinannya di folder Downloads Anda (*Quillbinder web pages/<judul>*), karena browser yang dipasang sebagai paket sandbox (misalnya Firefox di Kubuntu) tidak dapat membaca folder milik {{APP}} sendiri; baris di bawah tombol menyebutkan lokasi salinan itu, dan klik berikutnya menggantinya. Di Windows, salinan dibuat di folder sementara dan dihapus saat {{APP}} ditutup, sehingga tidak ada yang ditulis di luar folder program. **Buka Alamat Asli** membuka alamat halaman, sehingga Anda melihat halaman seperti hari ini; tombol ini mati bila halaman tidak memiliki alamat. Halaman web yang disimpan Scrivener (satu berkas *.mht* masing-masing) ditampilkan dengan cara yang sama; browser seperti Firefox tidak dapat membuka berkas *.mht* sendiri, sehingga salinannya berupa HTML biasa dengan gambar di sampingnya.

Dengan cara apa pun, alamat halaman menjadi **penanda** dokumen (inspektor, Penanda), deskripsi singkat halaman menjadi **sinopsis**-nya, dan **catatan** merekam alamat, situs, penulis, tanggal publikasi, dan kapan Anda mengambilnya.

**Menyeret tautan** dari browser web ke folder di binder, ke papan gabus, atau ke kerangka mengimpor halaman itu secara langsung, dengan pilihan yang Anda buat terakhir kali di jendela.

Perlu diketahui: halaman diambil seperti yang dilakukan browser, tetapi tanpa menjalankan skripnya. Halaman yang membangun teksnya dengan skrip sering kali tetap membawa teks itu dalam datanya, dan data itulah yang kemudian dipakai (tanpa pemformatan dan gambar; sebuah pesan menyatakannya). Halaman di balik login atau paywall memberikan apa yang ditampilkannya kepada pengunjung yang belum login. Halaman aman (https) memerlukan OpenSSL 3 di sistem Anda (bagian dari setiap Ubuntu). Alamat web yang mengarah ke berkas PDF tidak diimpor dengan cara ini: unduh berkasnya dan gunakan **Berkas > Impor > Berkas…**.

**Skenario.** Berkas Fountain (*.fountain*) dan berkas Final Draft (*.fdx*) diimpor sebagai skenario: paragraf mendapat elemen format skenario proyek Anda, dokumennya berada dalam mode skenario, dan halaman judul berkas masuk ke catatan dokumen. Teks tebal, miring, dan bergaris bawah, catatan, dan pemisah halaman paksa dipertahankan; nomor adegan, tanda revisi, dan tampilan dua kolom pada dialog ganda tidak (kedua ucapan itu berurutan). **Impor dan Pisahkan…** dengan **Judul adegan** memotong skenario menjadi satu dokumen per adegan, dinamai menurut judulnya.

### Mengimpor proyek Scrivener

**Berkas > Impor > Proyek Scrivener…** membaca proyek Scrivener 3 atau 2 (folder *.scriv*; pilih berkas *.scrivx* di dalamnya) dan membuat proyek {{APP}} baru darinya. Proyek Scrivener hanya dibaca, tidak pernah diubah. Sejak 0.92.0, laporan setelah impor juga menyebutkan setiap berkas yang tidak dapat disalin (gambar, PDF, berkas audio, gambar kartu indeks, peta pikiran), dan berkas semacam itu tidak dihitung sebagai telah diimpor; sebelumnya, berkas itu dihilangkan tanpa kabar.

Impor membawa: struktur binder, teks, sinopsis, catatan, label, status, kata kunci, metadata kustom, jenis bagian, penanda sertakan-dalam-kompilasi, snapshot, komentar dan catatan kaki, anotasi sebaris, berkas riset (gambar, PDF, dan lainnya), koleksi, penanda proyek, dan target. Pengaturan kompilasi tidak diimpor; atur sekali di jendela kompilasi. Pilih apakah proyek baru menyimpan teksnya sebagai Markdown atau RTF.

Judul dan gaya: Scrivener menandai teks bergaya di dalam berkasnya dengan kode seperti `<$Scr_Ps::0><$Scr_H::2>`. Kode itu bukan teks; {{APP}} menghapusnya. Judul mendapat **Heading 1** sampai **Heading 6**, dan paragraf dengan salah satu gaya paragraf Scrivener mendapat gaya proyek Anda dengan nama yang sama (**Title**, **Block Quote**; **Centered Text** milik Scrivener menjadi **Centered**). Sejak 0.81.3, teks dengan salah satu gaya karakter Scrivener mendapat gaya karakter proyek Anda dengan nama itu, dan gaya Scrivener yang tidak memiliki gaya bernama sama di proyek Anda menjadi gaya proyek, dengan tampilan yang diberikan Scrivener. Proyek yang diimpor oleh versi sebelum 0.14 mempertahankan kode sebagai teks; {{APP}} menghapusnya saat proyek semacam itu dibuka dan menyatakannya di bilah status. Penanda proyek tidak diimpor sebelum 0.14; untuk mendapatkannya, impor proyek Scrivener itu sekali lagi.

Jika ada yang tidak terbawa dengan benar, proyek aslinya tidak tersentuh dan Anda dapat menyalin teks langsung dari Scrivener.

### Bekerja dalam format Scrivener

Alih-alih membuat salinan, {{APP}} juga dapat mengerjakan proyek Scrivener 3 itu sendiri, sehingga Anda dapat melanjutkan di Scrivener nanti (misalnya di komputer lain).

- **Membuka**: **Berkas > Buka Proyek…** (**Buka Lainnya…** di jendela awal) dan pilih berkas *.scrivx* di dalam folder *.scriv*. Pertama kali, {{APP}} bertanya: **Bekerja dalam Format Scrivener** (perubahan Anda disimpan ke dalam *.scriv*), **Konversi…** (proyek {{APP}} tersendiri dibuat darinya, seperti pada impor di atas), atau **Batal**. Setelah itu, proyek terbuka tanpa pertanyaan, juga dari **Berkas > Buka Terbaru**.
- **Proyek baru**: pilih **Format: Proyek Scrivener 3 (.scriv)** di jendela Proyek Baru.
- **Menyimpan**: setiap penyimpanan (juga yang otomatis) menulis ke dalam *.scriv*. Hanya yang Anda ubah yang ditulis; dokumen yang tidak Anda sentuh tetap memakai berkas yang ditulis Scrivener. Judul jendela menyebutkan *(format Scrivener)*, dan **Berkas > Tampilkan Proyek di Pengelola Berkas** serta cadangan mengambil folder *.scriv*.
- **Proyek besar** (sejak 0.60.5): penyimpanan otomatis saat Anda menulis hanya menaruh teks, catatan, dan sinopsis yang Anda ubah ke dalam *.scriv* (dan binder, bila berubah), sehingga tetap singkat bahkan dengan ratusan ribu kata. Segala hal lainnya, seperti indeks pencarian Scrivener, snapshot, dan gambar, menyusul ketika Anda berpindah ke program lain, memakai **Berkas > Simpan** (**Ctrl+S**), atau menutup proyek; jadi sebelum membuka proyek di Scrivener, tutuplah di sini atau berpindahlah dulu ke program lain.
- **Satu program pada satu waktu**: jika Scrivener sedang membuka proyek (atau tidak ditutup dengan benar), {{APP}} menyatakannya dan bertanya sebelum membukanya. Tutup proyek di satu program sebelum membukanya di program lain. Bila proyek diubah di Scrivener (atau oleh layanan sinkronisasi) sejak terakhir kali Anda membukanya di sini, {{APP}} membacanya lagi dan memberi tahu di bilah status berapa dokumen yang berubah; dokumen yang diubah di Scrivener sementara itu tidak pernah ditimpa dengan teks yang lebih lama.

Yang masuk ke dalam *.scriv*: binder, teks (sebagai RTF, beserta gambarnya), sinopsis, catatan (juga untuk gambar, PDF, audio, dan video, sejak 0.81.3), label, status, jenis bagian, **Sertakan dalam Kompilasi**, penanda (proyek dan setiap dokumen, juga ke alamat web dan berkas), target proyek beserta tenggat, target dokumen, riwayat penulisan, papan gabus (susunan setiap folder dan posisi kartu yang diletakkan bebas), judul, penulis, dan bahasa berkas hasil kompilasi (dalam pengaturan kompilasi Scrivener), snapshot, komentar dan catatan kaki, anotasi dan catatan kaki sebaris, gaya paragraf dan karakter, kata kunci, kolom metadata kustom beserta nilainya, koleksi, pencarian tersimpan (koleksi pencarian, sejak 0.90.0), gambar kartu indeks (sejak 0.90.0), gambar, PDF, audio dan video, halaman web, ikon (menurut nama Scrivener; ikon Anda sendiri di folder *Icons*-nya), dan peta pikiran (sebagai berkas tambahan di samping berkas dokumen, yang dibiarkan oleh Scrivener). Semuanya ditulis dalam bentuk yang dipakai Scrivener 3 untuk Windows:

- Kata kunci, metadata kustom, dan koleksi mempertahankan nomor dan nama Scrivener (sejak 0.81.2), sehingga sama di kedua program.
- Gaya (sejak 0.81.3): gaya teks yang Anda ubah ditandai di dalamnya seperti yang ditandai Scrivener, sehingga Scrivener menampilkan gaya yang sama. Gaya yang Anda buat di {{APP}} ditambahkan ke gaya proyek Scrivener saat pertama kali sebuah teks memakainya, beserta tampilannya.
- Anotasi dan catatan kaki sebaris (sejak 0.81.3) tetap sebaris, anotasi dalam warnanya (sebelumnya, teks yang Anda ubah memberikannya kepada Scrivener sebagai komentar dan catatan kaki tertaut).
- Halaman web (sejak 0.81.3): halaman yang Anda simpan di {{APP}} masuk sebagai halaman web tersimpan milik Scrivener sendiri (satu berkas *.mht* beserta gambarnya) dan mempertahankan alamatnya. Berkas lain yang tidak memiliki jenis di Scrivener masuk sebagai dokumen teks yang menyebutkan berkasnya, dengan berkas itu di sampingnya; {{APP}} membacanya kembali seperti semula.
- Audio, video, dan halaman web yang oleh versi sebelum 0.81.2 diubah menjadi dokumen teks semacam itu menjadi dokumen audio, video, atau halaman web milik Scrivener sendiri lagi pada penyimpanan berikutnya.
- Teks (sejak 0.90.0) ditulis kembali dengan segala hal yang disimpan Scrivener di dalamnya: indentasi kanan di bawah nol, daftar beserta penomorannya, garis bawah tautan, gambar yang ditautkan ke berkas, font yang dinamai menurut wajahnya (seperti *Cambria-BoldItalic*), dan posisi gambar yang datanya hilang. Komentar dan catatan kaki mempertahankan ID-nya, apakah kotaknya terlipat, dan pemformatannya sendiri selama Anda tidak mengubahnya.
- Papan gabus (sejak 0.90.0): kartu yang Anda letakkan bebas mempertahankan posisinya di kedua program. Scrivener menyimpan posisi dalam lebar kartu; {{APP}} mengubahnya menjadi ukuran kartunya sendiri, sehingga susunannya tetap sama berapa pun ukuran kartu yang ditampilkan setiap program. Setiap folder mempertahankan susunannya (diletakkan bebas atau dalam kisi), seperti di Scrivener: memilih **Bebas** atau kisi untuk papan gabus sebuah folder diingat untuk folder itu.
- Label dan status (sejak 0.90.0): label dan status bawaan untuk dokumen baru, serta nama yang diberikan Scrivener kepada "No Label" dan "No Status", tetap seperti yang dimiliki proyek.
- Pencarian tersimpan (sejak 0.90.0): pencarian tersimpan Scrivener adalah koleksi pencarian di {{APP}}, dan koleksi pencarian yang Anda buat atau ubah masuk ke dalam *.scriv* sebagai pencarian tersimpan. Daftar hasil pencarian terbaru Scrivener tetap milik Scrivener. Diperiksa dengan pencarian tersimpan untuk salah satu kata; bagaimana Scrivener menamai jenis lainnya (semua kata, frasa persis, ekspresi reguler, hanya dokumen yang disertakan atau dikecualikan dari kompilasi) belum terlihat di proyek sungguhan, sehingga pencarian tersimpan jenis itu mungkin muncul di Scrivener sebagai pencarian untuk semua kata. Di mana dalam dokumen sebuah pencarian mencari bukan bagian dari yang ikut berpindah: pencarian tersimpan yang dibaca dari Scrivener mencari di mana saja.
- Gambar kartu indeks (sejak 0.90.0): kartu yang menampilkan gambar alih-alih sinopsisnya mempertahankannya di kedua program; melepaskannya atau memasangnya di {{APP}} sampai ke Scrivener.

Yang tetap hanya di sisi {{APP}} (dalam salinan kerja di *~/.local/share/{{APP}}/ScrivenerProjects*): format skenario, pengaturan kompilasi (kecuali judul, penulis, dan bahasa), objek (bagian 7), dan tata letak jendela; apa pun yang sudah dimiliki *.scriv* dari hal-hal ini dibiarkan seperti adanya.

Pertama kali proyek Scrivener dibuka atau diimpor, binder-nya menampilkan folder yang terbuka di Scrivener dan dokumen yang dipilih di sana (0.90.0; {{APP}} tidak pernah menulis pengaturan jendela milik Scrivener). Pertama kali Anda membuka proyek dalam format Scrivener dengan 0.81.2 atau lebih baru, {{APP}} membaca *.scriv* lagi dan menggabungkan kata kunci, metadata kustom, dan koleksi salinan kerjanya dengan milik proyek, sehingga tidak ada yang Anda buat dengan versi lama yang hilang; penyimpanan berikutnya memasukkannya ke *.scriv*. Pembukaan pertama dengan 0.90.0 atau lebih baru melakukan hal yang sama untuk penanda, target dokumen, tenggat, riwayat penulisan, dan papan gabus.

Jika penyimpanan ke dalam *.scriv* terputus (crash, disk penuh) setelah dokumen baru ditulis ke salinan kerja, pembukaan berikutnya mempertahankan dokumen itu dan menyatakannya; penyimpanan berikutnya memasukkannya ke *.scriv*. Dokumen yang dihapus di Scrivener tetap terhapus.

**Berkas > Simpan Sebagai…** (**Ctrl+Shift+S**) menyimpan proyek yang terbuka dengan nama lain atau di tempat lain, dan inilah cara proyek mengubah formatnya. Di dialog, ketik nama dan pilih tempat; **jenis berkas** menentukan formatnya: *Scrivener 3 Project (\*.scriv)* atau *{{APP}} Project (\*.qbx)* (nama yang berakhir dengan *.scriv* atau *.qbx* menentukannya sendiri). Proyek adalah sebuah folder: program membuatnya dari nama (*Name.scriv*, atau *Name.quillbinder* dengan *Name.qbx* di dalamnya). Nama yang sudah ada ditolak.

- Sejak saat itu jendela bekerja pada proyek baru, dan setiap penyimpanan berikutnya masuk ke sana. Judul proyek adalah nama yang Anda ketik. Perubahan yang belum Anda simpan hanya masuk ke proyek baru.
- Aslinya tetap berada di disk seperti pada saat itu (simpan sebagai salinan atau hapus); aslinya juga ada di **Berkas > Buka Terbaru**, di bawah proyek baru. Berkas kuncinya dihapus saat jendela meninggalkannya, dan proyek baru mendapat berkas kuncinya sendiri.
- Proyek baru adalah proyek tersendiri, dengan **cadangan**-nya sendiri: proyek ini memulai seri baru, dan cadangan yang dibuat sejauh ini tetap bersama aslinya (bagian 14, Cadangan). Proyek dalam format Scrivener mendapat pengenal baru di *.scrivx*-nya untuk itu.
- Simpan Sebagai bekerja dengan cara yang sama dari satu format ke format yang sama (nama atau tempat baru) dan ke format lainnya (peralihan yang dilakukan menu "Format Proyek" sampai 0.92).

**Berkas > Ekspor > Proyek Scrivener…** menulis **salinan** dalam format Scrivener, dan entri di bawahnya pada menu yang sama, **Proyek {{APP}}…**, salinan dalam format {{APP}} sendiri; keduanya membiarkan jendela pada aslinya: tidak ada yang berubah di proyek yang Anda kerjakan (sebelum 0.93.0 ini adalah "Simpan Salinan sebagai ..." di menu "Format Proyek").

### Mengekspor dokumen

**Berkas > Ekspor > Berkas…** menyimpan dokumen yang dipilih untuk program lain. Gunakan Kompilasi untuk naskah jadi; gunakan Ekspor untuk menyerahkan dokumen satu per satu kepada seseorang, atau untuk menyimpan salinan di luar {{APP}}.

1. Di binder, pilih dokumen dan folder yang akan diekspor.
2. Pilih **Berkas > Ekspor > Berkas…**.
3. **Ekspor berkas teks sebagai**: Rich Text (.rtf), Word (.docx), Markdown (.md), teks biasa (.txt), Fountain atau Final Draft (untuk skenario, Fountain sudah terpilih). Gambar, berkas PDF, halaman web, audio, dan video diekspor apa adanya; format itu hanya berlaku untuk catatannya.
4. **Metadata**: **Ekspor metadata** menulis berkas teks di samping setiap dokumen, "*nama* (metadata).txt", dengan judul, jenis, label, status, kata kunci, sertakan dalam kompilasi, jenis bagian, jumlah kata, target, tanggal pembuatan dan perubahan, metadata kustom, dan penanda. **Ekspor sinopsis** menambahkan sinopsis ke dalamnya (atau menulis "*nama* (synopsis).txt" bila metadata tidak diekspor).
5. **Opsi**: **Sertakan subdokumen** (folder menjadi folder), **Ekspor catatan** ("*nama* (notes)" dalam format langkah 3; teks biasa untuk skenario), **Ekspor komentar dan anotasi sebaris** (hapus centang untuk menyerahkan teks bersih; kata yang dikomentari tetap ada, catatan kaki selalu tetap ada), **Ekspor snapshot** (folder "*nama* (snapshots)" dengan satu berkas per snapshot, dinamai menurut tanggal dan judul).
6. **Lokasi**: folder tempat menyimpan. Bila beberapa dokumen dipilih, dokumen itu masuk ke folder baru yang namanya Anda berikan (judul proyek diusulkan; jika folder sudah ada, nomor ditambahkan).
7. Klik **Ekspor**. Folder terbuka di pengelola berkas.

Pilihan Anda diingat untuk ekspor berikutnya.

Cara cepat: **seret dokumen keluar dari binder** ke pengelola berkas atau ke desktop. Dokumen teks disimpan sebagai berkas RTF (atau Word, Markdown, atau teks biasa: **Berkas > Opsi > Perilaku**), gambar dan berkas lain apa adanya, dan folder menjadi folder dengan dokumennya di dalam. Jika pengelola berkas bertanya apakah akan memindahkan atau menyalin, keduanya tidak masalah: dokumen tetap berada di proyek Anda.

### Bandingkan dan transfer: berkas Word atau LibreOffice dan proyek

Ketika naskah kembali dari editor, atau Anda melanjutkan pekerjaan di Word atau LibreOffice, **Berkas > Bandingkan dan Transfer > Bandingkan dengan Berkas…** menemukan apa yang berbeda antara berkas itu dan proyek Anda dan mengambil alih sisi yang Anda pilih.

1. Pilih berkasnya: berkas Word (*.docx*) atau berkas LibreOffice (*.odt*). Berkas Word lama (*.doc*), RTF, teks biasa, dan Markdown juga dapat dibandingkan, tetapi hanya dibaca: teksnya dapat masuk ke proyek; berkasnya sendiri tidak dapat diubah.
2. {{APP}} membandingkan berkas dengan **apa yang dihasilkan Kompilasi dari proyek**, bukan dengan seluruh proyek: dokumen kelompok kompilasi (Draf, kecuali kelompok lain dipilih di jendela kompilasi) yang memiliki **Sertakan dalam Kompilasi**, menurut urutan binder. Bagian depan dan belakang, folder Riset, dan dokumen yang dikecualikan dari Kompilasi tidak dihitung. Jika kurang dari 65 % teks berkas ditemukan di dalamnya, {{APP}} menyatakannya dan bertanya apakah tetap akan membandingkan (**Tetap Bandingkan**); berkas itu mungkin milik proyek lain, atau Kompilasi menghilangkan apa yang dimiliki berkas. Sebuah kata dianggap ditemukan bila berada dalam rangkaian empat kata yang juga dimiliki teks hasil kompilasi, sehingga beberapa kata yang berubah tidak membuat berkas dianggap tidak berkaitan.
3. Jendela **Bandingkan dan Transfer** menampilkan perbedaannya. Pilih sisi yang diperbarui:
   - **Perbarui proyek dari berkas**: perubahan dilakukan di dokumen Anda. Sebelum itu, setiap dokumen yang berubah mendapat snapshot, "Before Compare and Transfer (*nama berkas*)" (**Dokumen > Snapshot**), sehingga Anda selalu dapat kembali. Teks yang hanya ada di berkas menjadi dokumen baru (dinamai menurut judulnya di berkas), ditaruh setelah dokumen sebelumnya.
   - **Perbarui berkas dari proyek**: berkas yang Anda pilih tetap seperti adanya. {{APP}} menulis salinan di sampingnya, "*nama* (Quillbinder).docx" (dengan nomor bila nama itu sudah dipakai). Di berkas Word, setiap perubahan adalah **perubahan terlacak** oleh penulis "Quillbinder": buka salinannya di Word atau LibreOffice dan terima atau tolak setiap perubahan di sana (di Word pada tab Review; di LibreOffice dengan Edit, Track Changes, Manage). Salinan LibreOffice (*.odt*) menerima perubahan secara langsung, tanpa tanda.
4. Di tampilan, teks **hijau** masuk, teks **merah** yang dicoret keluar, dan teks **biru** mendapat tebal, miring, atau garis bawah dari sisi lain. Paragraf abu-abu adalah teks yang tidak berubah di sekitar perubahan, untuk menunjukkan letaknya. Setiap tempat memiliki tautan **ambil** atau **lewati**; sebuah klik mengalihkannya.
5. Klik **Transfer ... ke dalam Proyek** atau **Tulis ... ke dalam Salinan Berkas**. Sebuah pesan menyatakan apa yang dilakukan; untuk berkas, **Buka Berkas** membuka salinannya.

**Per Bab** (**Berkas > Bandingkan dan Transfer > Per Bab**, juga kotak centang di jendela) mencantumkan dokumen di sebelah kiri, masing-masing dengan kotak centang dan apa yang berbeda ("2 tempat", "hanya di berkas", "tidak ada di berkas"). Hapus centang dokumen untuk membiarkannya seperti adanya; sebuah klik menampilkan perubahannya di sebelah kanan. Tanpa Per Bab, semua perubahan ditampilkan satu di bawah yang lain.

**Langkah demi Langkah** (**Berkas > Bandingkan dan Transfer > Langkah demi Langkah**, juga di jendela) menampilkan satu perubahan pada satu waktu: **Ambil** atau **Lewati**, atau kembali dengan **Kembali**. Di akhir, jendela menyatakan berapa banyak perubahan yang akan diambil; lalu klik Transfer. Kedua pilihan diingat untuk lain kali.

Yang dibandingkan: kata, serta tebal, miring, dan garis bawah. Setiap paragraf berkas dicocokkan dengan dokumen yang memiliki katanya, sehingga sebuah adegan ditemukan juga bila dipindahkan di berkas (adegan itu kemudian ditandai "di tempat lain dalam berkas"; urutan binder tidak diubah). Judul berkas yang bukan teks dokumen, seperti judul bab yang dibuat oleh Kompilasi, tidak dibandingkan.

Yang dicentang pada awalnya: setiap perubahan di dalam dokumen. Teks yang hanya ada di satu sisi dicentang bila memiliki setidaknya 100 kata (bab baru dari berkas menjadi dokumen baru; adegan yang tidak ada di berkas ditambahkan ke salinan). Satu dokumen utuh tidak pernah dipindahkan ke Tempat Sampah, dan satu bagian utuh tidak pernah diambil dari berkas, kecuali Anda mencentangnya.

Batasan: paragraf baru mengambil tampilan paragraf di sebelahnya. Teks dalam tabel dibandingkan seperti paragraf lain (tabelnya sendiri tidak pernah diambil), dan bidang dihitung bersama teks yang ditampilkannya. Header dan footer, kotak teks, serta teks catatan kaki dan komentar berkas tidak dibandingkan dan tetap seperti adanya; komentar dan catatan kaki proyek tetap di proyek dan tidak ditulis ke berkas. Jika berkas diubah setelah perbandingan, salinan tidak ditulis; bandingkan lagi.

### Mencetak

**Berkas > Cetak Dokumen Saat Ini…** (**Ctrl+P**) mencetak apa yang ditampilkan editor saat ini:

- **Teks** (satu dokumen, atau beberapa dalam teks gabungan): teks beserta pemformatan dan gambarnya, judul setiap dokumen di atas teksnya, dan catatan kaki di bagian bawah halaman. Bila papan ketik berada di pemegang salinan, dokumen pemegang salinan yang dicetak.
- **Papan gabus**: kartu indeks menurut urutan yang ditampilkan, sebanyak yang muat pada selembar (dengan garis untuk memotong), atau satu kartu pada setiap lembar untuk kertas kartu indeks.
- **Kerangka**: baris dan kolom seperti yang ditampilkan kerangka, dengan judul kolom di bagian atas setiap halaman.
- **Gambar** (sebesar yang diizinkan halaman), **berkas PDF** (halamannya), atau **halaman web** (teksnya).

Jendela cetak memilih printer, halaman, dan jumlah salinan. **Pilihan** hanya mencetak teks yang dipilih, atau hanya kartu indeks atau baris kerangka yang dipilih. Untuk menyimpan hasil cetak sebagai berkas PDF, pilih **Print to File (PDF)** sebagai printer.

**Berkas > Pratinjau Cetak** menampilkan halaman lebih dulu; tombol printer pada pratinjau mencetaknya, **Tutup** (atau **Esc**) menutup pratinjau.

**Berkas > Pengaturan Cetak…** menentukan bagaimana tampilan di atas kertas. Jendela terbuka pada tab untuk apa yang ditampilkan editor; **Pratinjau…** menampilkan hasilnya sebelum Anda mengklik **OK**. Pengaturan ini milik proyek; centang **Pakai pengaturan ini untuk proyek baru juga** untuk menjadikannya standar bagi proyek baru.

- **Halaman**: ukuran dan orientasi kertas (tombol **Kertas Tampilan Halaman** mengambil alih kertas dan margin **Tampilan > Pengeditan Teks > Ukuran dan Margin Halaman…**), margin, serta header dan footer, masing-masing dengan bagian kiri, tengah, dan kanan. Placeholder: `<$title>` (apa yang dicetak), `<$projecttitle>`, `<$author>`, `<$date>`, `<$p>` (nomor halaman), dan `<$pages>` (jumlah halaman). Hapus centang **Header dan footer di halaman pertama juga** untuk halaman pertama yang bersih.
- **Teks**: apa yang dicetak bersama teks (judul setiap dokumen, sinopsis di bawahnya, catatan dokumen setelah teks, komentar dan anotasi sebaris sebagai catatan di bagian bawah halaman, catatan kaki di akhir teks alih-alih di bagian bawah setiap halaman, sorotan); font (seperti di editor, atau **Satu font** pilihan Anda); dan jarak baris (seperti di editor, tunggal, 1,5 baris, atau ganda, dengan ruang untuk koreksi). Dokumen skenario mempertahankan tata letak skenarionya.
- **Kartu Indeks**: ukuran kartu (3 x 5 in, 4 x 6 in, 5 x 8 in, A7, A6, atau ukuran Anda sendiri), sebanyak yang muat pada selembar atau satu kartu pada setiap lembar, apa yang ditampilkan setiap kartu (judul, sinopsis, baris pertama teks bila tidak ada sinopsis, gambar, warna label sebagai garis di bagian atas, status, kata kunci, garis kartu indeks, garis di sekeliling setiap kartu), dan font.
- **Kerangka**: sinopsis (seperti yang ditampilkan kerangka, selalu, atau tidak pernah), juga baris di dalam grup tertutup, judul kolom di bagian atas setiap halaman, garis di antara baris, titik warna label, dan font. Dengan banyak kolom, **Lanskap** di tab **Halaman** memberi lebih banyak ruang.

Cetak Dokumen Saat Ini mencetak dokumen seperti saat Anda mengerjakannya. Untuk naskah jadi dengan tampilan format kompilasi, kompilasikan (bagian 13) lalu cetak berkas PDF atau Word-nya.

---

## 15. Cadangan

{{APP}} membuat cadangan (berkas ZIP dari seluruh proyek) setiap kali Anda menutup proyek, dan menyimpan lima yang terakhir per proyek. Ubah ini di **Berkas > Opsi > Cadangan**, tempat Anda juga dapat memilih folder cadangan (sebaiknya di drive lain, atau folder yang disalin oleh program sinkronisasi Anda).

Sejak 0.91, cadangan semua proyek berada langsung di folder cadangan, masing-masing dinamai menurut proyeknya beserta tanggal dan waktu. Sejak 0.92.0, nama itu juga memuat **ID** proyek, lima karakter setelah "#" (*My Novel #K7T2M bak 2026-11-20 15-40-00.zip*). ID milik proyek, bukan komputer: ID sama di setiap komputer tempat proyek dibuka, dan tetap ada ketika Anda mengganti nama proyek atau memindahkan foldernya. Cadangan sebuah proyek adalah berkas yang memuat judul dan ID-nya, dan hanya berkas itu yang dihitung dan dibersihkan untuknya, sehingga dua proyek dengan judul yang sama mempertahankan cadangannya masing-masing dan jumlah yang dipertahankan masing-masing. Salinan atau proyek baru yang dibuat program sendiri adalah proyek tersendiri dengan ID-nya sendiri: **Berkas > Simpan Sebagai…** dan entri Proyek {{APP}} di **Berkas > Ekspor**, proyek yang dipulihkan dengan **Pulihkan dari Cadangan** (proyek itu juga mendapat judul sendiri), dan proyek yang dibuat dari templat Anda sendiri. Salinan yang dibuat di luar program (folder yang Anda salin dengan tangan, cadangan yang Anda ekstrak dengan tangan) memiliki ID aslinya: beri salinan semacam itu judul sendiri. Proyek dalam format Scrivener mengambil ID-nya dari pengenal yang disimpan Scrivener di *.scrivx*. Jika folder cadangan Anda berada di OneDrive sebuah organisasi, administratornya mungkin melarang "#" dalam nama berkas; OneDrive untuk rumah mengizinkannya. Cadangan yang dibuat oleh versi sebelum 0.92.0 tidak memiliki ID dalam namanya: tidak pernah dihapus oleh pembersihan, **Pulihkan dari Cadangan** tetap mencantumkannya, dan Anda menghapusnya sendiri bila tidak lagi membutuhkannya. Bila Anda mengubah judul proyek, cadangan yang dibuat dengan judul lama tetap ada sampai Anda menghapusnya. Versi 0.70 sampai 0.90 menyimpan cadangan setiap proyek di folder tersendiri di dalam folder cadangan (*My Novel 1a2b3c4d*); folder semacam itu dibiarkan apa adanya, tidak ada lagi yang ditambahkan atau dihapus di sana, dan **Pulihkan dari Cadangan** tetap mencantumkan cadangannya. Cadangan berjalan di samping jendela: setelah beberapa saat, jendela kemajuan menunjukkan sejauh mana prosesnya dan dapat membatalkannya. Siapa yang membuka proyek (berkas kunci dengan nama komputer dan login) tidak ada dalam cadangan.

- Dengan **Cadangkan pada setiap penyimpanan manual** (**Berkas > Opsi > Cadangan**), setiap **Berkas > Simpan** (**Ctrl+S**) juga membuat cadangan. Dua cadangan dalam detik yang sama mendapat nomor ("... (2)"). Bila cadangan itu tidak dapat dibuat, tidak lengkap, atau harus menunggu di luar folder cadangan, pemberitahuan di samping teks menyatakannya (0.92.0; sebelumnya, hanya bilah status yang menyatakannya, selama beberapa detik). Teks Anda tetap tersimpan. Pemberitahuan muncul sekali per penyebab sampai Anda keluar dari program; bilah status menyatakannya pada setiap penyimpanan.
- **Baris pesan** di bawah binder menampilkan pesan singkat satu per satu, selama 5 detik (10 dengan tautan) atau sampai Anda mengkliknya; pesan lain menunggu. Pengaya juga memakainya (misalnya "Model unloaded, RAM usage minimal" ketika Asisten Penerjemah atau Transkripsi Suara membebaskan modelnya setelah beberapa waktu tidak dipakai). Selama sebuah model AI dikeluarkan atau dimuat, juga pada pemuatan pertama, sebuah baris tipis di dasar inspektor menyatakan "Perubahan model sedang berlangsung" sampai model baru siap; bila sebuah model tidak dimuat karena tidak akan muat di memori, baris itu menyatakan alasannya (lihat **Berkas > Opsi > Umum > Model AI**). Baris itu tidak memakan ruang selama kosong; selagi inspektor disembunyikan, baris di bawah binder menampilkan kedua teks ini. **Bantuan > Pemecahan Masalah dan Pengembangan > Informasi Sistem** menyebutkan memori yang dipegang program.
- **Pengaya yang tidak terpasang.** **Edit > Alat Menulis** mencantumkan "Asisten Penerjemah" dan "Speech Transcription" selama pengayanya hilang atau dimatikan; masing-masing menampilkan pemberitahuan dengan tombol yang membuka **Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya**.
- **Berkas > Cadangkan > Cadangkan Sekarang** membuat cadangan seketika. Sejak 0.94.0, keberhasilannya disampaikan di baris pesan di bawah binder ("Backup saved." dengan tautan **Tampilkan Folder**, yang membuka folder cadangan; tooltip menyebutkan berkasnya; pesan dengan tautan bertahan 10 detik); tidak ada kotak yang menunggu OK. Kotak hanya muncul bila ada yang gagal, atau bila cadangan dibuat tetapi sebagian berkas tidak dapat dibaca atau cadangan lama tidak dapat dihapus.
- **Cadangan yang gagal dibuat karena shutdown atau perintah kill** dicatat dan dibuat pada pembukaan berikutnya (0.81). Sejak 0.93.0, catatan bertahan sampai cadangan itu dibuat (di folder cadangan, atau menunggu di sampingnya seperti dijelaskan di atas): pembukaan yang cadangannya gagal meninggalkan catatan untuk pembukaan berikutnya. **Cadangan lama yang tidak dapat dihapus oleh pembersihan** (berkas yang terkunci atau dilindungi) tetap ada, dan sebuah pesan menyebutkan berkas dan alasannya satu kali; hapus dengan tangan. Selagi Anda terus menulis, pesan ini berupa pemberitahuan kecil di samping teks yang tidak mengambil tombol apa pun.
- **Cadangkan ke…** menyimpan cadangan di mana pun yang Anda pilih, misalnya di flash disk USB.
- **Pulihkan dari Cadangan…** (0.70) mencantumkan cadangan proyek yang terbuka beserta tanggal, ukuran, dan apakah lengkap; pilih satu dan **Pulihkan dan Buka**: cadangan diekstrak ke folder baru di samping proyek, *… (restored 2026-11-10 14-03)*, judulnya berbunyi "(restored …)", dan terbuka di sana. Proyek yang Anda buka tidak diubah. Daftar memuat cadangan dengan ID proyek ini, yang tanpa ID (dibuat sebelum 0.92.0) dan milik proyek lain dengan judul yang sama; nama berkas di daftar menunjukkan mana yang mana, sehingga tidak ada yang tidak terjangkau. Sejak 0.93.0, daftar juga mencantumkan cadangan yang menunggu di folder kerja program sendiri (dibuat saat folder cadangan tidak dapat dijangkau; cadangan berikutnya ke folder cadangan menaruhnya di sana): cadangan itu ditandai "waiting (not yet in the backup folder)" dan dipulihkan dengan cara yang sama. Sejak 0.81, pemulihan berukuran berapa pun berfungsi (cadangan 2 GB atau lebih, berkas lebih dari 1 GB seperti video, lebih dari 65.535 berkas), setiap berkas diperiksa saat ditulis, pemulihan yang tidak muat di drive tidak dimulai, jendela kemajuan dapat membatalkannya, dan bila berkas tidak dapat dipulihkan, pesan menyebutkan masing-masing beserta alasannya (disk penuh, entri yang rusak). Sejak 0.93.0, pesan memiliki tiga bentuk: **dipulihkan** (semuanya ada), **dipulihkan tidak lengkap** (dengan judul demikian, dengan tanda peringatan, berkas yang hilang disebutkan beserta alasannya: lihat proyek yang dipulihkan sebelum Anda mengandalkannya) dan **tidak dipulihkan**; dan bila judul proyek yang dipulihkan tidak dapat ditandai "(restored …)", pesan menyatakannya.
- **Pindahkan Cadangan Lama ke Folder Cadangan…** (0.93) memindahkan cadangan yang disimpan versi 0.70 sampai 0.90 di folder tersendiri di dalam folder cadangan (*My Novel 1a2b3c4d*) ke folder cadangan itu sendiri, dengan nama aslinya, sehingga berada bersama yang lain. Perintah ini hanya menyala selama proyek yang terbuka memiliki folder semacam itu berisi cadangan. Perintah ini bertanya lebih dulu dan menyebutkan berapa banyak yang ada; cadangan yang namanya sudah ada di folder cadangan tidak pernah ditimpa dan tetap di tempatnya; laporan menyatakan apa yang dipindahkan dan apa yang ditinggalkan (beserta alasannya), dan folder lama dihapus bila tidak ada yang tersisa di dalamnya. Cadangan ini tidak memiliki ID dalam namanya, sehingga pembersihan tidak pernah menghitung atau menghapusnya, seperti sebelumnya; **Pulihkan dari Cadangan** mencantumkannya di kedua tempat.
- **Tampilkan Folder Cadangan** membuka folder cadangan.

**Tempat cadangan disusun (sejak 0.90.3).** Cadangan dikemas dan diperiksa di folder milik program sendiri, *~/.local/share/{{APP}}/backup-work* (di Windows *data\backup-work* di folder program), dan hanya berkas yang sudah jadi yang ditaruh di folder cadangan, dalam satu langkah dan dengan nama akhirnya: dipindahkan ke sana bila kedua folder berada di drive yang sama, jika tidak disalin langsung ke nama akhirnya dan dibandingkan dengan aslinya. Jadi folder cadangan yang diawasi program sinkronisasi, atau yang berada di flash disk USB atau drive jaringan, tidak pernah berisi cadangan setengah jadi atau berkas sementara, dan cadangan yang terputus tidak pernah tampak seperti yang sudah selesai. (Sebelum 0.90.3, cadangan ditulis di dalam folder cadangan sebagai *… .zip.part* dan diganti namanya di sana.) Folder kerja kosong di antara cadangan, selain catatan kecil tentang folder cadangan yang dilayaninya.

**Bila folder cadangan tidak terjangkau** (drive yang tidak terpasang, folder yang tidak dapat dibuat atau ditulisi, tidak ada ruang), cadangan tetap dibuat dan menunggu di folder kerja. Sebuah pesan menyatakannya dan menyebutkan kedua folder, dan cadangan proyek berikutnya menaruh cadangan yang menunggu pada tempatnya sebelum cadangannya sendiri. Sebanyak jumlah cadangan yang dipertahankan itulah yang menunggu. **Cadangkan ke…** ke folder yang tidak dapat ditulisi hanya menyatakannya beserta alasannya; pilih folder lain.

**Salinan yang dibuat program sinkronisasi.** Hanya berkas yang dinamai seperti {{APP}} menamai cadangannya (*Title #K7T2M bak 2026-11-20 07-56-13.zip*, dengan "(2)" atau "(incomplete)" bila berlaku) yang dihitung sebagai cadangan dan dapat dihapus oleh pembersihan. Berkas lain di folder cadangan proyek dibiarkan dan tidak dihitung: misalnya salinan yang dibuat program sinkronisasi saat bermasalah dengan sebuah berkas (*… -MyComputer-safeBackup-0001.zip*, *… (conflicted copy).zip*), atau salinan yang Anda buat dengan tangan. (Sebelum 0.90.3, salinan semacam itu dihitung sebagai cadangan pada tanggalnya, dan beberapa di antaranya dapat mendorong keluar cadangan lama yang sebenarnya.) **Pulihkan dari Cadangan…** tetap mencantumkannya, sehingga Anda dapat memulihkan dari sebuah salinan; hapus sendiri salinan yang tidak Anda perlukan.

Proyek besar (lebih dari 4 GB, misalnya dengan audio dan video) juga dicadangkan dengan benar (sejak versi 0.20). Berkas yang tidak dapat dibaca (misalnya karena izinnya, atau berkas folder tersinkron yang belum ada di komputer ini) dihilangkan; cadangan tetap dibuat, tetapi dinamai *… (incomplete).zip*, sebuah pesan menyebutkan berkas yang hilang (juga untuk cadangan otomatis saat proyek dibuka atau ditutup, 0.65), dan tidak ada cadangan lama yang dihapus karenanya: jumlah cadangan yang dipertahankan hanya menghitung cadangan yang lengkap. Cadangan mana yang tertua ditentukan oleh tanggal dan waktu dalam namanya, sehingga mengganti nama proyek tidak mengubah cadangan mana yang dipertahankan (0.81; sebelumnya, judul baru yang urutannya sebelum judul lama membuat setiap cadangan baru dihitung sebagai yang tertua, dan langsung dihapus). Setelah pembersihan, program memeriksa bahwa cadangan baru masih ada. Dari cadangan yang tidak lengkap, yang terbaru hingga jumlah yang dipertahankan tetap ada (0.81). Folder di proyek yang merupakan tautan ke folder di tempat lain dicadangkan beserta berkasnya (0.81); tautan yang menunjuk ke mana-mana, atau kembali ke dalam proyek, membuat cadangan tidak lengkap. Jika program diakhiri oleh sinyal (shutdown yang tidak menunggu, perintah kill) saat **Buat cadangan saat proyek ditutup** menyala, tidak ada waktu untuk cadangan; saat proyek dibuka berikutnya, cadangan dibuat dan bilah status menyatakannya (0.81). Proyek disimpan sebelum setiap cadangan; jika penyimpanan itu gagal (disk penuh, folder yang tidak dapat ditulisi), tidak ada cadangan yang dibuat dan sebuah pesan menyatakan alasannya, sehingga cadangan tidak pernah kekurangan perubahan terbaru Anda tanpa Anda ketahui. Cadangan proyek dalam format Scrivener memuat *.scriv* dan, di sampingnya, salinan kerja {{APP}} (*… - working copy of {{APP}}*), dengan kata kunci, gaya, objek, dan hal-hal lain yang hanya diketahui {{APP}}.

**Untuk memulihkan cadangan dengan tangan** (misalnya di komputer lain): buka folder cadangan, ekstrak berkas ZIP (klik kanan > Extract), dan buka proyek di dalamnya dengan **Berkas > Buka Proyek**.

**Enkripsi: mengunci proyek dan cadangan (0.99.0, Berkas > Opsi > Enkripsi).** {{APP}} dapat mengunci berkas sebuah proyek dan cadangannya sehingga hanya kunci Anda yang membukanya (OpenPGP, format yang dipakai Kleopatra, Gpg4win, dan GnuPG). Fitur ini mati sampai Anda membuat kunci. **Buat Kunci** adalah wizard singkat: wizard mengusulkan frasa sandi berupa enam kata acak (Anda boleh mengetik frasa sandi sendiri; yang lemah ditolak), membuat kunci Anda dan sebuah kunci pemulihan, lalu menulis folder *Quillbinder Keys yyyy-mm-dd* di tempat pilihan Anda, sebaiknya flash disk USB. Folder proyek, folder cadangan, dan folder program sinkronisasi (OneDrive, Dropbox, Nextcloud, Google Drive, iCloud, Syncthing, pCloud, MEGA) ditolak. Folder itu berisi *my-key-secret.asc* (dikunci oleh frasa sandi Anda), *my-key-public.asc*, *recovery-key-secret.asc* (dikunci oleh frasa sandinya sendiri), *recovery-key-public.asc*, dan *README.txt* dalam bahasa program, tanpa rahasia di dalamnya. **Frasa sandi pemulihan** ditampilkan sekali di layar: tulis di kertas. Frasa ini tidak disimpan di mana pun, sehingga flash disk saja atau kertas saja tidak membuka apa pun. Wizard lalu menanyakan dua dari keenam kata, membaca kembali berkas yang ditulis dan membuka berkas uji dengan setiap kunci, dan menawarkan salinan kedua folder itu di tempat lain.

Di halaman itu, **Impor Kunci** mengambil folder kunci di perangkat lain (pilih folder, ketik frasa sandi sekali), **Impor Berkas Kunci** mengambil berkas kunci milik Anda sendiri (misalnya yang diekspor dari Kleopatra; hanya kunci versi 4 pada Curve25519, jenis lain ditolak dengan pesan), **Ekspor Kunci** menulis folder itu lagi dari perangkat ini, dan **Lupakan frasa sandi di perangkat ini** menghapus frasa sandi yang diingat (disimpan di penyimpanan kunci sistem: KWallet, Secret Service, Windows Credential Manager) dan mengunci kunci sampai Anda mengetiknya lagi. Kedua sakelar milik proyek yang terbuka: **Cadangan terenkripsi** dan **Proyek terenkripsi**; yang kedua ditampilkan abu-abu untuk proyek dalam format Scrivener, karena Scrivener tidak dapat membukanya.

**Apa yang dilakukan kedua sakelar.** Dengan **Cadangan terenkripsi** menyala, setiap cadangan proyek berupa berkas *.zip.gpg* yang dikunci oleh kunci Anda dan kunci pemulihan; komputer ini hanya memerlukan kunci publik untuk membuatnya, sehingga komputer yang dicuri atau folder sinkronisasi yang disalin tidak membukanya. Ini juga berfungsi untuk proyek dalam format Scrivener. Cadangan yang dibuat sebelumnya tetap seperti semula, dan cadangan biasa tetap dapat dipulihkan; jumlah cadangan yang disimpan menghitung kedua jenis. **Pulihkan dari Cadangan** meminta frasa sandi Anda bila kunci terkunci. Dengan **Proyek terenkripsi** menyala, setiap berkas di folder proyek dikunci sendiri-sendiri (teks, catatan, berkas proyek, snapshot, versi yang disimpan, gambar dan media, pengaturan), sehingga program sinkronisasi dan kunci yang menyebutkan nama komputer bekerja seperti sebelumnya; hanya kunci itu dan sebuah berkas kecil *encryption.json* (sidik jari kedua kunci, tidak ada yang rahasia) tetap dapat dibaca. Menyalakan atau mematikannya menutup proyek, mengonversi setiap berkas (sebuah jendela menampilkan kemajuan), dan membukanya lagi; bila komputer berhenti di tengah, setiap berkas adalah yang lama atau yang baru, proyek tetap terbuka, dan pembukaan berikutnya menyelesaikan pekerjaan itu. Cadangan sebuah proyek terenkripsi selalu terenkripsi. Proyek dibuka dengan frasa sandi yang diingat di komputer ini; jika tidak, {{APP}} menanyakannya sekali. Di komputer tanpa kunci Anda, program menyatakannya dan menawarkan **Impor Kunci**, dan tidak mengubah apa pun di proyek. Berkas yang rusak disebutkan namanya, dokumennya tetap kosong dan tidak pernah ditimpa, dan sisanya terbuka. Selama proyek terbuka, gambar, berkas PDF, dan media ditampilkan dari salinan tidak terkunci di folder privat di memori komputer, yang dihapus saat proyek ditutup. Proyek terenkripsi tidak dapat disimpan dalam format Scrivener (matikan dulu enkripsinya). Nama berkas dan folder tidak disembunyikan, dan versi {{APP}} sebelum 0.99.0 tidak dapat membuka proyek terenkripsi.

**Membuka berkas Anda tanpa {{APP}}.** Cadangan terenkripsi adalah berkas bernama seperti *My Novel #K7T2M bak 2026-11-27 09-30-00.zip.gpg*: berkas ZIP yang dikunci oleh kunci Anda. Dengan **Kleopatra** (Linux) atau **Gpg4win** (Windows): File > Import, pilih *my-key-secret.asc*; lalu File > Decrypt/Verify, pilih berkas *.zip.gpg*, ketik frasa sandi, dan buka berkas ZIP yang keluar. Dengan perintah **gpg**: `gpg --import my-key-secret.asc`, lalu `gpg --output backup.zip --decrypt backup.zip.gpg`, lalu `unzip backup.zip`. Untuk proyek terenkripsi, cadangan adalah jalan keluar yang lebih sederhana: berkasnya satu, dan setelah dekripsi berupa berkas ZIP biasa berisi proyek. Kunci pemulihan membuka berkas yang sama: impor *recovery-key-secret.asc* dan ketik frasa sandi pemulihan. Di **Android** jalan keluarnya adalah PC: salin cadangan ke sana dan pakai Kleopatra atau gpg (OpenKeychain tidak lagi dikembangkan).

**Apa yang dilindungi enkripsi, dan apa yang tidak.** Enkripsi melindungi berkas di disk, di cadangan, dan di salinan: orang yang mendapatkan berkasnya, laptop yang dicuri, flash disk yang disalin, atau layanan sinkronisasi hanya melihat berkas terkunci. Enkripsi tidak melindungi proyek selagi terbuka (teks ada di memori komputer dan dapat ada di papan klip), dan siapa pun yang masuk sebagai Anda dapat membuka proyek yang frasa sandinya diingat {{APP}}. **Jika kedua kunci hilang, atau kedua frasa sandi, teksnya hilang: tidak ada pintu belakang.**

**Aturan yang menjaga proyek terenkripsi tetap terenkripsi (0.99.0).** **Simpan Sebagai** dan **Simpan Salinan** sebuah proyek terenkripsi menghasilkan proyek terenkripsi (salinan memiliki kedua kunci yang sama). Sebuah proyek hanya terbuka di komputer yang memiliki bagian rahasia dari salah satu kuncinya **dan bagian publik dari keduanya**: komputer yang hanya memiliki *my-key-secret.asc* dan tidak punya kunci pemulihan menyatakannya dan menawarkan **Impor Kunci** (impor *recovery-key-public.asc* di sana), karena setiap penyimpanan dienkripsi untuk kedua kunci. Bila berkas kecil *encryption.json* tidak dapat dibaca, atau tidak ada padahal berkas proyek terenkripsi, proyek tidak dibuka dan tidak ada cadangannya yang dibuat sampai berkas itu kembali (ambil versi sebelumnya dari cadangan atau dari riwayat program sinkronisasi Anda); tidak ada yang ditulis selama itu. Berkas sebuah proyek terenkripsi yang byte-byte pertamanya rusak dianggap rusak (dokumennya tetap kosong dan tidak pernah ditimpa), dan berkas yang ditulis komputer lain dengan cara lama setelah Anda mematikan enkripsi ditampilkan terdekripsi bila kunci Anda terbuka, jika tidak dibiarkan. **Proyek terenkripsi** tidak dapat dinyalakan atau dimatikan selagi komputer lain atau jendela {{APP}} lain membuka proyek itu: tutup dulu di sana. Selama proyek terenkripsi terbuka, berkas sementara pratinjau kompilasi, kompilasi melalui program lain, gambar yang dijatuhkan ke lembar, dan ekspor untuk diseret masuk ke folder privat yang sama di memori komputer seperti gambar yang tidak terkunci, bukan ke folder sementara sistem.

**Apa yang tetap ditampilkan proyek terenkripsi.** Nama berkas dan folder (berupa angka), ukuran, dan waktunya. Salinan konflik, salinan penyimpanan yang terputus, dan salinan darurat (**Simpan Teks di Tempat Lain** dan apa yang ditulis ketika {{APP}} tidak dapat menyimpan) dinamai menurut nomor dokumen, bukan judulnya; folder salinan darurat dinamai menurut tanggalnya, dan *README.txt*-nya serta catatan awal berikutnya tidak menyebut judul maupun proyek (folder proyek ditemukan dari tanggalnya). Pembelajaran tulisan tangan program tidak menyimpan apa pun selama proyek terenkripsi terbuka. Salinan terdekripsi untuk penampil dan teks di memori komputer adalah bagian "tidak terlindungi selagi terbuka" di atas. **Menyalakan enkripsi tidak menarik kembali apa yang sudah tersimpan dalam bentuk biasa**: versi-versi sebelumnya yang disimpan layanan sinkronisasi dalam riwayatnya, cadangan biasa yang lebih lama (hapus sendiri bila Anda ingin menghilangkannya), dan salinan di folder Unduhan Anda yang dibuat oleh **Buka Halaman Tersimpan** tetap seperti semula.

**Pemeriksaan cadangan terenkripsi.** Setelah cadangan ditulis, {{APP}} mendekripsinya lagi dan membandingkannya dengan berkas ZIP bila kunci Anda terbuka di komputer ini. Bila tidak (proyek dengan **Cadangan terenkripsi** menyala yang di komputer ini hanya dibuatkan cadangan, dan frasa sandinya tidak pernah ditanyakan di sini), pemeriksaannya hanya bahwa berkas itu utuh (paketnya berakhir tepat di akhir berkas); pemeriksaan itu tidak dapat mengetahui byte yang berubah. Untuk proyek dalam format Scrivener, **Cadangan terenkripsi** adalah pengaturan setiap komputer: nyalakan di setiap komputer yang membuat cadangannya.

---

## 16. Opsi

**Berkas > Opsi** (**Ctrl+,**) memiliki deretan ikon di bagian atas untuk setiap bagian, daftar halaman (atau tab) di sebelah kiri, dan opsinya di sebelah kanan. Tidak ada yang berubah sampai Anda mengklik **OK** atau **Terapkan**. Di kiri bawah, **Kelola** mengekspor profil yang dipakai ke sebuah berkas dan mengimpor satu (**Ekspor Profil...**, **Impor Profil...**, misalnya untuk membawa pengaturan Anda ke komputer lain; berkas opsi yang lebih lama (*.qboptions*) dapat diimpor sebagai profil; lihat "Profil" di bawah), membuka **Kelola Profil...**, menyimpan tampilan saat ini sebagai berkas tema (berkas yang sudah ada diganti hanya bila yang baru telah ditulis sepenuhnya; jika penulisan gagal, berkas lama tetap seperti semula dan pesan menyatakannya), dan mencantumkan tema; **Bawaan** mengembalikan setiap opsi ke standar (proyek terbaru, tema, tata letak, bilah alat, dan pintasan Anda sendiri tetap ada).

**Tempat setiap opsi disimpan.** Arahkan penunjuk ke sebuah opsi untuk melihat tempat nilainya disimpan: tooltip-nya menyatakan apakah proyek yang terbuka yang menyimpannya atau program menyimpannya untuk semua proyek, dan apakah tempat itu **tetap** (pengaturan yang hanya dapat dipegang program, seperti bahasa) atau **bergantung pada pengaturan** **Pengaturan untuk** di bagian atas (dan, pada halaman Penampilan, **Tampilkan dan ubah tampilan**). (0.90.1 menampilkan P hijau atau A biru sebelum setiap opsi; 0.90.2 menyerahkannya kepada tooltip.) Pada layar kecil, halaman dapat digulir, sehingga OK, Batal, dan Terapkan tetap terlihat.

Sejak 0.70, jendela terbuka untuk **Semua proyek** (pengaturan program); pilih **Proyek ini** di bagian atas untuk menetapkan sesuatu hanya untuk proyek yang terbuka. Pengaturan milik proyek dapat membuat penyimpanan lebih aman tetapi tidak pernah kurang aman: penundaan simpan otomatis yang lebih singkat yang dipakai, cadangan yang dibuat program tetap menyala, jumlah cadangan yang lebih besar dipertahankan, dan keluar otomatis hanya milik program. Penyimpanan terbaru terjadi paling lama 10 detik setelah perubahan, berapa pun penundaan simpan otomatisnya.

- **Umum**: **Startup** (membuka kembali proyek terakhir, dialog berkas desktop, **Bahasa** program, mulai dari start berikutnya: English, Deutsch, Español, Français, Português (Brasil), Italiano, Русский, Bahasa Indonesia, Türkçe, Polski atau Nederlands (sembilan yang baru adalah terjemahan mesin, ditandai demikian dalam daftar sampai seorang penutur memeriksanya; panduan pengguna, panduan mulai cepat, dan proyek selamat datang mengikuti bahasanya, dalam bahasa Inggris bila suatu bahasa belum memilikinya; start pertama memakai bahasa sistem bila termasuk salah satunya, dan halaman start pertama menawarkan daftar yang sama); **Kelompokkan digit angka besar**: jumlah kata, target, dan statistik dengan pemisah di antara ribuan, seperti sistem Anda menulis angka (44,825 atau 44.825), atau 44825 bila tidak dicentang); **Menyimpan** (interval simpan otomatis, jumlah proyek terbaru, snapshot pada setiap Berkas > Simpan); **Model AI** (satu tempat untuk setiap fungsi AI: Transkripsi Ucapan, Asisten Penerjemah, Baca Nyaring, dan Tulisan Tangan: **Inti CPU untuk AI**, bawaannya tiga perempat jumlah inti dibulatkan ke bawah, pada komputer dengan 6 inti atau kurang jumlah inti dikurangi satu, dan paling sedikit satu, dengan tombol **Bawaan**; **Keluarkan model setelah [10] menit tidak dipakai**; dan satu baris dengan memori yang terpakai: RAM komputer, setiap kartu grafis yang dilaporkan mesin, dan model yang dimuat beserta ukurannya. Kedua pengaturan milik komputer ini, bukan milik proyek. Aturannya: model yang tidak dipakai selama waktu itu dikeluarkan dari memori. Sebelum sebuah model dimuat, bila lebih dari separuh memori tempatnya dimuat terpakai (oleh semua yang ada di komputer), model Quillbinder yang paling lama tidak dipakai dikeluarkan lebih dulu; bila model baru tetap akan menaikkan memori itu melebihi 80 %, model tidak dimuat, baris pesan di bawah inspektor menyatakan alasannya, dan fungsinya menawarkan model yang lebih kecil berikutnya atau prosesor sebagai gantinya. Bila sistem sendiri melaporkan kekurangan memori, model yang tidak dipakai langsung dikeluarkan. Pekerjaan AI berjalan dengan prioritas rendah, sehingga komputer tetap dapat dipakai); **Tulisan Tangan** (halaman tersendiri, lihat *Tulisan Tangan* di bawah); **Baca Nyaring** (halaman tersendiri, lihat *Baca Nyaring* di bawah); **Informasi Penulis** (nama, alamat, telepon, dan email Anda untuk halaman judul, dipakai oleh placeholder `<$author>`, `<$street>`, `<$city>`, `<$state>`, `<$zip>`, `<$country>`, `<$phone>`, `<$email>`, `<$address>`); opsi alat menulis (**Catatan Sementara**); **Templat Bersama** (folder tambahan untuk templat proyek, misalnya yang disinkronkan di antara komputer Anda; templatnya muncul di Berkas > Proyek Baru); **Sitasi** (program yang dijalankan **Format > Daftar Pustaka/Sitasi...**, misalnya Zotero atau JabRef); **Pemisah** (apa yang berada di antara dokumen yang digabungkan dan di antara catatan sementara yang ditambahkan ke dokumen: satu return, baris kosong, pemisah halaman, tidak ada, atau teks Anda sendiri); **Keluar Otomatis** (simpan dan tutup setelah beberapa menit tanpa masukan tombol atau tetikus); **Peringatan** (**Tampilkan Konsol Log Internal** menampilkan pesan program pada sesi ini, **Atur Ulang Semua Peringatan** bertanya lagi pertanyaan yang dijawab dengan "Don't ask again").
- **Pengeditan**: **Opsi** (muat kembali zoom terakhir editor, zoom teks bawaan setiap area, sentimeter atau inci dan langkah penggaris, tombol Insert untuk menimpa, format tanggal Anda sendiri untuk Sisipkan > Tanggal Saat Ini, apa yang ditampilkan penghitung langsung, anotasi sebaris tanpa warna, komentar dan catatan kaki yang berhenti sebelum tanda baca, titik sisip berbentuk blok); **Pemformatan** (pemformatan dan gaya proyek baru); **Revisi** (lima warna Format > Mode Revisi, dari palet bersama).
- **Perilaku**: **Mode Menulis** (layar, jendela utama menyingkir, Esc); **Tautan Dokumen** (tempat tautan dokumen terbuka, tautan balik); **Klik Ganda** (klik ganda di papan gabus membuat kartu); **Seret & Jatuhkan** (dokumen yang dijatuhkan ke dokumen teks, format teks yang diseret ke program lain); **Folder & Berkas** (folder terbuka sebagai papan gabus; teks dengan subdokumen terbuka seperti folder); **Pemutaran** audio dan video (bagian 6); **Referensi Cepat** (panel dibuka kembali bersama proyeknya); **Tombol Return** (Return mengakhiri pengeditan sinopsis; Return membuat kartu atau baris baru); **Snapshot** (pesan saat snapshot dibuat).
- **Penampilan**: tema, dan sebelas bagian jendela (Antarmuka Umum, Binder, Mode Menulis, Papan Gabus, Kartu Indeks, Inspektor & Catatan, Editor Utama, Kerangka, Teks Gabungan, Bilah Kemajuan Target, Tanda Tekstual), masing-masing dengan opsi, font, dan warnanya, serta **Warna Standar** (sejak 0.95: keenam label yang dipakai proyek baru di awal, keenam warna palet warna teks dan palet sorot, dan warna awal kedua tombol; **Bawaan** mengembalikan standar program; proyek yang sudah ada mempertahankan labelnya). Lihat "Tema, warna, dan font" di bawah. **Antarmuka Umum** juga memiliki jalur lengkap proyek di bilah judul dan ukuran ikon bilah alat.
- **Koreksi**: koreksi saat mengetik (bagian 4) dan **Ejaan** (kamus, kata dengan angka, garis bawah, daftar kata pribadi).
- **Papan Ketik**: pintasan setiap perintah (bagian 19).
- **Berbagi**: **Impor** (komentar, catatan kaki, gaya, pengodean teks biasa, berkas HTML; bagian 14), **Ekspor** (format berkas dan catatan Berkas > Ekspor > Berkas), **Konversi** (bagaimana berkas *.doc* dibaca).
- **Cadangan**: kapan mencadangkan (saat membuka, saat menutup, pada setiap Berkas > Simpan), berapa banyak cadangan yang dipertahankan (instalasi baru dimulai dengan 25 yang terbaru; kolomnya hanya berisi angka, kata-katanya berada di sekelilingnya: "Simpan [25] cadangan terbaru dari setiap proyek"), dan di mana.
- **Ekstra**, untuk tambahan milik {{APP}} sendiri: **Peta Pikiran** (opsi papan, bentuk dan tampilan catatan baru, latar papan tanpa warna sendiri pada tema terang dan gelap, ukuran ikon pada catatan dan garis), **Objek** (apakah proyek baru mendapat templat objek Anda sendiri atau yang standar), **Pengaya dan Plugin** (format kompilasi dan alat menulis yang terpasang, folder plugin), **Laporan Crash** (setelah berapa detik tanpa jawaban laporan pembekuan ditulis, berapa banyak laporan yang disimpan).
- **Ucapan** (dengan pengaya Transkripsi Suara): model, perangkat, bahasa, mikrofon, dan jeda (lihat bagian 10, *Transkripsi Suara*). Alat menulis dari pengaya yang lebih baru mungkin juga membawa bagian tersendiri.

Jendela lain program dapat membuka opsi pada sebuah halaman, misalnya halaman Pemutaran dari pemutar media.

Pembaruan dan pengaya memiliki jendelanya sendiri: **Bantuan > Pengaya dan Pembaruan > Instal Pembaruan atau Pengaya…** dan **Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya…** (bagian 17).

Pengaturan khusus proyek (label, status, jenis bagian, tampilan teks, metadata kustom, folder khusus, judul dan penulis untuk kompilasi) ada di **Proyek > Pengaturan Proyek**.

### Pengaturan untuk satu proyek

Di bagian atas **Berkas > Opsi**, **Pengaturan untuk:** memilih tempat penyimpanan apa yang Anda ubah:

- **Semua proyek (program)**: pengaturan program, untuk setiap proyek. Di sinilah jendela terbuka.
- **Proyek ini**: proyek yang terbuka menyimpan apa yang Anda ubah di sini sebagai pengaturannya sendiri (di *settings/options.ini* pada foldernya). Berlaku selama proyek itu terbuka; apa pun yang tidak ditetapkan proyek berasal dari pengaturan program. Judul jendela berbunyi "Opsi: Proyek Ini", dan baris di bawah pilihan menyatakan berapa banyak pengaturan milik proyek (klik angkanya untuk melihatnya). **Pakai Pengaturan Program** menghapus semuanya; pengaturan program berlaku lagi untuk proyek. Perubahan yang dilakukan di luar opsi, misalnya pintasan yang diubah di jendela papan ketik atau bilah alat yang disusun, masuk ke proyek bila proyek sudah memiliki nilainya sendiri untuk itu, dan ke program jika tidak.

Beberapa pengaturan hanya milik program dan tidak ditampilkan untuk proyek: proyek terbaru, membuka kembali proyek terakhir, dialog berkas desktop, posisi jendela dan tata letak, waktu laporan crash, dan tampilan untuk proyek baru. Tampilan proyek selalu miliknya sendiri: halaman **Penampilan** menampilkan tampilan proyek yang terbuka atau tampilan untuk proyek baru (**Tampilkan dan ubah tampilan** di bagian atasnya); dengan **Proyek ini** dipilih di atas, halaman hanya menampilkan tampilan proyek. **Bawaan** dan **Kelola > Impor Profil…** hanya bekerja pada pengaturan program, jadi pilih **Semua proyek** untuk keduanya.

**Pengaturan mana yang didahulukan** (**Umum > Startup**, untuk semua proyek): bila dicentang (standar), pengaturan milik proyek didahulukan dan program mengisi sisanya. Bila tidak dicentang, pengaturan program selalu berlaku: pengaturan milik proyek tidak dipakai (tetap berada di proyek untuk nanti), dan setiap proyek menampilkan tampilan untuk proyek baru.

**Proyek > Simpan Pengaturan Proyek ke Berkas…** menyertakan opsi milik proyek (bagian **Opsi Proyek Ini**), sehingga dapat dibawa ke proyek lain.

### Tema, warna, dan font

Tema menetapkan warna, dan font bila diinginkan, setiap area jendela. Pilih satu di **Tampilan > Tema**, atau dengan tombol tema di bilah alat Tema (nyalakan bilah alat di **Tampilan > Bilah Alat**); perubahan langsung berlaku, tanpa memulai ulang. Sembilan tema bawaan tersedia:

| Tema | Tampilan |
|-------|------|
| Shelin's Dark Theme | jendela dan binder gelap, kertas hangat dengan teks hitam, Cinzel untuk judul teks gabungan (sejak 0.95; berada di urutan teratas daftar dan dapat dihapus di Kelola Tema; setelah itu tidak muncul lagi setelah pembaruan) |
| Default (Follow the Desktop) | warna desktop Anda (skema warna KDE) |
| Light | jendela abu-abu terang, kertas putih |
| Dark Mode | abu-abu gelap, dengan kartu indeks gelap |
| Sepia | krem hangat dan cokelat |
| Slate | jendela abu-abu, kertas putih di atas meja abu-abu |
| Midnight | biru gelap dengan aksen kuning amber |
| Ocean | biru-hijau terang |
| High Contrast | putih dan kuning di atas hitam |

Pengaya dapat membawa lebih banyak tema (pengaya "Extra themes" memiliki Northern Night, Forest, Graphite, Warm Paper, dan Rose Dawn).

**Tombol tema** (0.90.2, diubah di 0.91 dan 0.94). Satu tombol, **Tema**, di bilah alat Tema, yang pada awalnya mati (**Tampilan > Bilah Alat > Tema** menampilkannya; sebelum 0.94, empat tombol dan sebuah daftar menghabiskan sepertiga bilah alat utama). Menunya mencantumkan:

- **Light** dan **Dark**: sebuah klik langsung menampilkan tema itu, apa adanya.
- **My Look 1** dan **My Look 2** menyimpan dua tampilan Anda sendiri. Memilih yang kosong (menunya menyatakan *empty*) menyimpan tampilan yang Anda lihat sekarang (tema beserta setiap warna dan font yang Anda ubah di atasnya) di dalamnya; memilih yang terisi mengembalikan tampilannya. **Simpan Tampilan Saat Ini sebagai My Look 1** dan **...sebagai My Look 2**, di akhir menu, menyimpan tampilan yang Anda lihat sekarang di sana (keduanya bertanya sebelum mengganti sebuah tampilan). Kedua tampilan itu adalah tema biasa: ada di daftar di bawah dan di **Tampilan > Tema**, tempat keduanya juga dapat diekspor atau dihapus (yang dihapus meninggalkan entrinya kosong).
- **Semua tema**, di bawahnya, dengan yang sedang dipakai bertanda centang. Memilih satu melakukan apa yang dilakukan **Tampilan > Tema**: proyek yang terbuka dan proyek baru memakainya, dan pertanyaan apakah akan mempertahankan perubahan Anda sendiri di atasnya muncul seperti sebelumnya.
- **Tampilan Anda sendiri tidak pernah hilang karena sebuah pilihan.** Menu menampilkan tema apa adanya, sehingga apa yang Anda lihat benar-benar berubah. Jika warna atau font Anda sendiri berada di atas tema yang ditampilkan, semuanya akan hilang karenanya, sehingga disimpan lebih dulu: ke My Look yang kosong tanpa pertanyaan (bilah status menyebutkan tempatnya), dan bila keduanya terisi, {{APP}} bertanya apakah akan menyimpan di My Look 1, di My Look 2, atau beralih tanpa menyimpan.

Seperti setiap kontrol bilah alat, tombol tema dapat dipindahkan atau dilepas di bawah **Tampilan > Bilah Alat > Sesuaikan Bilah Alat**, dan seluruh bilah alat Tema dapat dinyalakan atau dimatikan di bawah **Tampilan > Bilah Alat**.

**Halaman pada tema gelap** (0.70). Halaman standar {{APP}} adalah kertas zaitun terang di samping meja abu-abu. Pada tema gelap, halaman menjadi abu-abu sangat gelap dengan teks putih (atau halaman gelap milik tema itu sendiri, bila ada), sehingga tema gelap tidak meninggalkan halaman terang di dalam bingkai gelap. Warna halaman yang Anda pilih sendiri (klik kanan > Penampilan, atau Berkas > Opsi > Penampilan) tetap berlaku di setiap tema.

**Mengubah warna dan font.** **Berkas > Opsi > Penampilan** (atau **Tampilan > Tema > Warna dan Font…**) mencantumkan bagian jendela di sebelah kiri. Masing-masing menampilkan opsi, font, dan warnanya (klik pada warna menampilkan palet bersama: warna label, enam warna standar, dan **Lainnya...**):

| Bagian | Font | Warna |
|------|-------|--------|
| Antarmuka Umum | menu, tombol, dan panel | jendela, Latar Inspektor (daftar dan kolom teks inspektor), baris selang-seling, teks, tombol, pilihan, tautan, tooltip, batas, teks yang diredupkan |
| Binder | binder | latar, judul |
| Mode Menulis | (tidak ada) | latar, kertas, teks |
| Papan Gabus | (tidak ada) | papan gabus, stempel status |
| Kartu Indeks | judul kartu, teks kartu | kartu indeks, judul, teks, garis di bawah judul, garis bergaris |
| Inspektor & Catatan | sinopsis | Strip Inspektor (baris tab, judul, baris Label dan Status), label, kartu dan teks sinopsis, catatan beserta teksnya |
| Editor Utama | judul bilah header | kertas, teks, ruang di samping kolom teks, meja di sekeliling halaman (hanya ditampilkan di tampilan halaman), bilah header (normal, editor aktif, dikunci di tempat) |
| Kerangka | kerangka | latar, baris selang-seling, teks |
| Teks Gabungan | judul di teks gabungan | (tidak ada) |
| Bilah Kemajuan Target | (tidak ada) | sesi, draf, target tercapai |
| Tanda Tekstual | (tidak ada) | tautan di teks, hasil pencarian, garis bawah ejaan, tanda Tampilkan Karakter Tak Terlihat |

**Setiap proyek mempertahankan tampilannya sendiri.** Tema binder, editor, papan gabus, kerangka, inspektor, dan mode menulis, warna dan font Anda sendiri untuk semuanya, lebar kolom teks, dan batas di samping teks adalah milik proyek: mengubahnya di satu proyek membiarkan proyek lain apa adanya, dan pembaruan {{APP}} tidak pernah mengubah tampilan proyek yang sudah ada. Daftar **Tampilkan dan ubah tampilan** di bagian atas **Berkas > Opsi > Penampilan** memilih tampilan siapa yang ditampilkan dan diubah halaman ini: **Proyek ini** (yang terbuka; dipilih pertama) atau **Proyek baru** (tampilan awal proyek baru). Berpindah di antara keduanya mempertahankan apa yang Anda ubah pada masing-masing sampai Anda menekan **Terapkan** atau **OK**. Warna menu dan panel (**Antarmuka Umum**) dan font menu, tombol, dan panel sama di setiap proyek. Proyek yang dibuka pertama kali oleh versi 0.40.2 atau lebih baru mempertahankan tampilan yang dimilikinya sampai saat itu. **Tampilan > Tema** menetapkan tema proyek yang terbuka dan proyek baru sekaligus.

**Menu klik kanan.** Di mana pun Anda mengklik kanan, menu memiliki submenu **Penampilan** dengan warna dan font dari apa yang berada di bawah tetikus: pada teks dan kertas di sampingnya yaitu kertas, teks, ruang di samping kolom, tautan, hasil pencarian, garis bawah ejaan, lebar kolom, dan batas; pada bilah header di atas editor yaitu warna dan fontnya; di binder yaitu latar, judul, font, indentasi, dan warna labelnya; di papan gabus yaitu gabus, kartu indeks, serta fontnya, stempel status, lebar kartu; di kerangka yaitu latar, baris, teks, dan fontnya; di inspektor dan catatannya yaitu strip, label, kartu sinopsis, dan catatan, serta Latar Inspektor, yang sama di setiap proyek; di mode menulis yaitu latar, kertas, dan teks. Warna menampilkan palet bersama dan **Warna Tema**; **Opsi Penampilan...** membuka halaman opsi ini. Seperti opsi, menu ini mengubah tampilan proyek yang terbuka (baris pertama submenu menyatakannya), atau, tanpa proyek, tampilan proyek baru.

Klik warna untuk mengubahnya, tombol font untuk memilih font. Gambar kecil di bagian atas menunjukkan hasilnya sebelum Anda menekan **Terapkan** atau **OK**. Perubahan Anda berada di atas tema: entri yang diubah ditampilkan tebal, tombol **↺** di sampingnya mengembalikannya ke tema, dan **Atur Ulang Warna dan Font ke Tema** melakukannya untuk semuanya. Warna yang belum Anda ubah dan tidak ditetapkan tema mengikuti yang lain; misalnya, binder memakai warna daftar sampai Anda memberinya warna sendiri. **Latar Inspektor** hanya mewarnai daftar dan kolom teks inspektor; kolom di bilah alat, penggaris, dan daftar opsi berwarna jendela dan menu.

Font dan ukuran teks yang Anda tulis bukan bagian dari tema: keduanya milik proyek (**Proyek > Pengaturan Proyek > Pemformatan**, bagian 4).

**Tema Anda sendiri.** **Simpan sebagai Tema…** di opsi (atau **Tampilan > Tema > Simpan Tampilan Saat Ini sebagai Tema…**) menyimpan tema beserta perubahan Anda dengan nama baru. Tema itu kemudian muncul di **Tampilan > Tema** seperti yang lain. **Tampilan > Tema > Kelola Tema…** mencantumkan semua tema: **Pakai Tema Ini**, **Duplikat…** (juga untuk tema bawaan, untuk mendapat titik awal milik Anda sendiri), **Ganti Nama…** dan **Hapus** (hanya tema Anda sendiri), **Ekspor…** (menulis berkas *.qbtheme* yang dapat Anda teruskan), dan **Impor…** (menambahkan berkas semacam itu).

Ketika Anda memilih tema lain saat memiliki perubahan sendiri, {{APP}} bertanya apakah akan mempertahankannya bersama tema baru atau memakai tema apa adanya.

Tautan di teks Anda disimpan dengan satu warna biru; pada kertas gelap, ditampilkan dalam biru yang lebih terang (**Editor Utama > Tautan di teks**). Teks yang Anda warnai sendiri mempertahankan warnanya di setiap tema.

**Kontras tautan.** Setiap tema bawaan menjaga warna tautannya (**Tautan di teks**, **Tautan di antarmuka**) pada kontras paling sedikit 4,5 : 1 (WCAG 2.x) pada setiap kertas dan latar belakang tempat tautan itu ditampilkan (kertas editor, kartu indeks, sinopsis, catatan, mode menulis, serta daftar dan panel antarmuka); warna tautan yang akan turun di bawahnya digeser sedikit, ke arah lebih gelap atau lebih terang, dan setiap area teks mendapat warna tautan yang cocok dengan kertasnya sendiri (kertas catatan yang gelap di samping kertas editor yang terang memiliki tautan sendiri yang lebih terang), juga pada kertas zaitun standar (sebelum 0.99.0 kontrasnya 4,05 : 1). Warna yang Anda pilih sendiri tidak pernah diubah: bila warna Anda sendiri turun di bawah 4,5 : 1 (tautan, atau teks pada kertasnya), **Berkas > Opsi > Penampilan** menampilkan catatan di bawah daftar perubahan yang menyebutkan pasangan terburuk dan rasionya. Catatan itu tidak memblokir apa pun.

### Profil

Tema hanyalah tampilan: warna dan font. **Profil** (sejak 0.95) menyimpan semua pengaturan program lainnya: bahasa, semua opsi, bilah alat beserta posisinya, ukuran dan posisi jendela, panel yang ditampilkan, pembatas, set papan ketik dan pintasan Anda, pencadangan dan folder cadangan, pemeriksaan pembaruan, warna standar, tata letak tersimpan di **Tampilan > Tata Letak**, dan profil menyebutkan temanya, sehingga beralih profil juga mengganti tampilan. Nama dan alamat Anda, nama komputer ini, proyek terbaru, dan posisi proyek selamat datang bukan bagian dari profil.

- **Beralih.** **Tampilan > Profil** mencantumkan profil; satu klik langsung beralih, tanpa dimulai ulang (bahasa adalah satu-satunya yang memerlukan start ulang; program memberitahukannya). Profil yang Anda tinggalkan disimpan lebih dulu. Bilah alat **Profil** (**Tampilan > Bilah Alat > Profil**, awalnya mati) menampilkan profil yang dipakai; klik pada tombolnya mencantumkan profil. Pengaturan milik proyek sendiri (**Pengaturan untuk: Proyek ini**) tetap berlaku di atas profil. Proyek yang terbuka menampilkan tampilan profil yang baru, seperti saat Anda memilih tema di **Tampilan > Tema**.
- **Default.** Program yang sudah pernah dipakai mengambil pengaturannya sebagai profil **Default** saat diperbarui ke 0.95, sehingga tidak ada yang hilang. Instalasi baru dimulai dengan pengaturan standar program, dalam tema terang atau gelap sesuai pengaturan sistem Anda bila sistem memberitahukannya. Halaman start pertama mengisi **Default**. **Default** tidak dapat dihapus.
- **Profil baru** dari pengaturan seperti sekarang: **Tampilan > Profil > Simpan Pengaturan sebagai Profil Baru…** (atau **Kelola Profil… > Simpan Pengaturan sebagai Profil Baru…**). Profil baru menjadi profil yang dipakai. **Kelola Profil…** juga mengganti nama, menyalin, dan menghapus profil (profil yang dipakai tidak dapat dihapus; beralihlah dulu).
- **Shelin's** adalah profil yang disertakan bersama program. Profil ini berada di urutan teratas daftar, menetapkan Sitka untuk teks baru beserta temanya sendiri, dan dapat dihapus; setelah itu tidak muncul lagi setelah pembaruan. Font yang disebutnya tetapi tidak terpasang (Cinzel, Sitka) digantikan font lain. Pilihan mesin pemutarannya memerlukan pengaya Format Video dan Audio Tambahan; pilihan model alat menulis hanya berlaku bila pengaya dan model itu terpasang.
- **Ukuran di layar lain.** Lebar binder, editor, dan inspektor disimpan sebagai bagian dari jendela; profil yang dibuat di layar besar menampilkan bagian yang sama di layar yang lebih kecil atau tablet (kolom samping mempertahankan lebar yang nyaman dibaca), dan jendela dibawa masuk ke dalam layar.
- **Ekspor dan impor.** **Kelola Profil… > Ekspor…** (atau **Berkas > Opsi > Kelola > Ekspor Profil…**) menulis profil ke satu berkas (*.qbprofile*) beserta tata letaknya dan, bila profil itu milik Anda sendiri, temanya. **Impor…** menambahkan berkas semacam itu sebagai profil baru (tidak langsung dialihkan ke sana). Berkas dapat menyebutkan program, folder, berkas, atau alamat; seperti berkas lain yang berasal dari orang lain, Anda ditanya sebelum semuanya diterima (folder yang tidak ada di komputer ini tidak berguna). Profil yang tidak disimpan oleh program ini di komputer ini (berkas yang disalin atau disinkronkan ke folder profil) menanyakan program, folder, berkas, dan alamatnya dengan cara yang sama saat Anda beralih ke profil itu. Berkas opsi yang lebih lama (*.qboptions*) juga dapat diimpor sebagai profil.

**Proyek > Simpan Pengaturan Proyek ke Berkas…** tetap seperti semula.

---

### Baca Nyaring

**Baca Nyaring** membacakan teks Anda dengan suara yang berjalan di komputer Anda. Fungsi ini tertanam di {{APP}} pada setiap sistem dan mati sampai Anda menyiapkannya: tidak ada yang dipasang dan tidak ada yang dimuat sebelum itu. Suaranya hanya diputar, tidak pernah disimpan ke berkas. Baca Nyaring hanya memerlukan **Mesin AI** dan sebuah model suara (keduanya dipasang dari **Berkas > Opsi > Baca Nyaring**); tidak perlu pengaya lain, karena mesin memutar suara sendiri melalui sistem suara komputer (ALSA, PulseAudio, atau PipeWire di Linux, WASAPI di Windows, AAudio atau OpenSL ES di Android).

**Bilah alat.** **Tampilan > Bilah Alat > Baca Nyaring** menampilkan atau menyembunyikan bilah alat **Baca Nyaring**; bilah ini ditampilkan sejak awal di Linux dan Windows dan disembunyikan di Android. Isinya **Putar/Jeda** (satu tombol), **Berhenti**, dan tombol kecil **Suara** dengan panah. Tombolnya mengikuti **Tampilan > Bilah Alat > Ukuran Bilah Alat**. **Putar** membacakan teks yang ditandai; bila tidak ada yang ditandai, membaca dari kursor sampai akhir teks. Kalimat yang sedang dibaca disorot di editor dan editor menggulir ke sana. Mengetik di teks menghentikan pembacaan, karena tempat-tempatnya tidak lagi cocok. **Klik pertama pada Putar, dan setiap klik selama belum ada mesin atau model suara yang terpasang, membuka Berkas > Opsi > Baca Nyaring.**

**Menu Suara** memiliki tiga kelompok dengan judul: **Suara** (Perempuan 1 sampai 3, Laki-laki 1 sampai 3), **Gaya** (Tenang, Netral, Aksi) dan **Kecepatan** (Lambat, Sedang, Cepat), dan di bagian bawah **Pengaturan Baca Nyaring…**. Tooltip tombol menampilkan pilihan Anda ("Perempuan 2 · Tenang · Sedang"). Pilihan yang tidak dapat dibuat oleh model suara yang dipakai ditampilkan abu-abu, dan tooltipnya menyebutkan alasannya: Pocket TTS tidak memiliki pengaturan gaya, jadi kelompok Gaya abu-abu; suara yang tidak dimiliki model ditampilkan abu-abu; suara-suara Qwen3-TTS abu-abu dengan "Suara belum terpasang." sampai tersedia. Pilihan disimpan untuk komputer ini. **Kecepatan** meregangkan suara yang sudah jadi dengan nada tetap: Sedang adalah laju model sendiri, Lambat dan Cepat kira-kira 15 persen lebih lambat dan lebih cepat.

**Berkas > Opsi > Baca Nyaring** memiliki: **Bahasa** (bahasa-bahasa program; Bahasa Indonesia, Türkçe, Polski, dan Nederlands ditampilkan abu-abu, "belum ada model untuk bahasa ini"); **Model** (Otomatis, atau satu model; masing-masing menampilkan ukuran unduhannya, bahasanya, dan, di komputer ini, "muat di <kartu grafis>", "muat di RAM" atau, dengan abu-abu, "tidak muat (perlu ...)"); **Perangkat** (Otomatis: kartu grafis bila model muat di memori kosongnya, jika tidak memori dan prosesor komputer; sebuah kartu grafis; atau prosesor); **uji kecepatan** (saat Anda memilih model, sebuah kalimat uji singkat mengukur seberapa cepat model berbicara di perangkat ini; bila lebih lambat dari waktu nyata, halaman menyatakannya, dan Baca Nyaring lalu membuat beberapa kalimat lebih dulu sebelum memutar); bagian yang memasang mesin dan model (sama seperti untuk fungsi AI lain); dan petunjuk ke **Umum > Model AI** untuk jumlah inti dan waktu sebuah model tetap dimuat. Otomatis memilih model dengan suara terbaik yang terpasang, muat, dan mampu mengimbangi waktu nyata: Pocket TTS bila mendukung bahasanya, jika tidak Qwen3-TTS 0.6B, jika tidak 1.7B.

**Memori dan kecepatan.** Baca Nyaring mengikuti aturan yang sama dengan fungsi AI lain (model dikeluarkan setelah menit yang ditetapkan di Umum > Model AI, dan tidak dimuat bila akan menaikkan memori melebihi 80 persen; baris pesan di bawah inspektor menyatakannya). Fungsi ini bekerja dengan prioritas rendah dan hanya membuat beberapa kalimat lebih dulu dari yang sedang diputar, tidak pernah satu bab utuh.

### Tulisan Tangan

**Tulisan Tangan** memungkinkan Anda menulis dengan pena atau jari dan membuat tintanya dibaca menjadi teks. Fungsi ini tertanam di {{APP}} pada setiap sistem. Fungsi ini memerlukan model tulisan tangan (mesin dan model dipasang dari **Berkas > Opsi > Tulisan Tangan**); sampai ada yang terpasang, penggunaan pertama tombolnya membuka halaman itu. Bahasa Rusia belum memiliki model.

**Tombol.** Di Linux dan Windows tombol ini mula-mula disembunyikan: nyalakan di **Tampilan > Bilah Alat > Tombol Tulisan Tangan** atau di **Berkas > Opsi > Tulisan Tangan** (di Android tombol ditampilkan sejak awal). Tombol ini ada di bilah Alat dan memiliki empat tampilan: pena abu-abu (mati), pena di atas halaman (editor), pena di panel samping (inspektor), dan pena melintasi keduanya (keduanya). **Satu ketukan** beralih antara mati dan mode yang terakhir Anda pakai; **tekan lama** (klik kanan, atau tekan dan tahan) membuka keempat pilihan: Mati, Editor, Inspektor, Keduanya. Mode disimpan untuk perangkat ini, bukan di proyek.

**Ketiga mode** berbagi satu area tulis.
- **Inspektor**: tab inspektor baru, **Tulisan Tangan**, adalah bantalan tulis dengan kisi. Inspektor terbuka bila sebuah mode memerlukannya.
- **Editor**: lapisan tembus pandang terletak di atas teks editor; Anda menulis di atas teks, dan teks tetap terlihat di bawah tinta. Inspektor terbuka pada tab Tulisan Tangan, tempat teks yang Anda tulis dikumpulkan.
- **Keduanya**: lapisan terletak di atas editor, garis antara editor dan inspektor, dan bagian bantalan inspektor. Garis itu tidak dapat diseret selama mode ini menyala. Kisi hanya ada di atas bagian bantalan.

Di Android papan ketik di layar tetap turun selama sebuah mode menyala; **Spasi**, **Backspace**, dan **Enter** adalah tombol di bawah bantalan.

**Menulis dan menunjuk.** Dengan pena, pena menulis; ketukan singkat pena menaruh kursor teks di sana; tombol samping pena memilih teks; jari menggulir (dan ketukan jari menaruh kursor). Tanpa pena, satu jari menulis; ketukan menaruh kursor; tekan lama dan seret memilih; dua jari menggulir. **Mencoret kata** (gerakan bolak-balik cepat di atasnya) menghapusnya: tinta yang belum dibaca, jika tidak kata-kata teks di bawah coretan, jika tidak sebuah kata di strip. Di bagian bantalan, ketukan singkat adalah titik tinta (titik pada huruf "i"), seperti di samping tinta yang baru saja Anda tulis.

**Membaca tinta.** Setelah jeda (mula-mula satu detik), baris yang Anda tulis dibaca. Setiap garis bantu pada kisi adalah satu baris; tanpa baris, tinta yang berjauhan dibaca sebagai baris-baris. Tinta bertahan sekitar 3 detik lalu memudar selama sekitar 2 detik; saat memudar, teks yang terbaca muncul di **strip** di bagian bawah inspektor. Tidak ada yang masuk ke teks Anda dengan sendirinya.

**Strip.** Ketukan pada sebuah kata menampilkan bacaan lainnya; pilih satu untuk mengganti kata itu. Menulis di atas sebuah kata dengan pena atau jari mengoreksinya. **Sisipkan** menaruh teks strip pada kursor teks yang Anda tetapkan sebelumnya (kursor itu bergerak bersama teks bila teks berubah) dan memindahkan tempat ke belakangnya. **Sematkan** membuat teks menempel pada pena atau jari: ketuk bidang teks (editor, catatan, sinopsis) untuk menampilkan tempatnya; tanda centang pada pin menjatuhkan teks di sana, tanda silang membatalkan dan teks kembali ke strip. **Kosongkan** membuang teks.

**Apa yang dikenali.** Sebuah baris tinta dibaca oleh model yang berjalan di komputer Anda (PP-OCRv6 small, Apache License 2.0, 13 MB) dengan ONNX Runtime dari **Mesin Tulisan Tangan**; tidak ada yang meninggalkan komputer. Model ini membaca tulisan beraksara Latin, cetak maupun tulisan tangan, dalam bahasa Inggris, Jerman, Spanyol, Prancis, Portugis, Italia, Indonesia, Turki, Polandia, dan Belanda, dengan huruf dan aksen bahasa-bahasa itu; bahasa Rusia belum memiliki model (pilihannya abu-abu). Setiap baris dibaca sendiri: tinta satu baris kisi adalah satu baris, dan tanpa baris, goresan yang bagian tengahnya dekat dengan kotak baris sejauh ini menjadi bagiannya. **Daftar kata** setiap bahasa (kata-kata yang paling sering dipakai beserta frekuensinya, dibuat dari wordfreq 3.1.1 oleh Robyn Speer, Creative Commons Attribution-ShareAlike 4.0; diunduh bersama model, satu untuk setiap bahasa) kemudian memperbaiki kata-kata yang diberikan oleh karakter: dari kata-kata dalam daftar yang dekat dengan yang terbaca, yang cocok dengan bentuk tinta dan umum yang menang; nama atau kata karangan yang tidak ada kata dalam daftar yang lebih cocok tetap seperti yang terbaca; kata yang menyatukan dua kata dipisah bila tidak ada satu kata pun yang cocok. Setiap baris dibaca dengan bahasa utama dan bahasa cadangan, dan bacaan yang lebih cocok dipertahankan. Ketukan pada sebuah kata di strip menampilkan kandidat berikutnya dan bacaan mentahnya. Tanpa daftar kata untuk sebuah bahasa, karakter dipakai seperti yang terbaca.

**Memasang.** Tombol **Instal** di **Berkas > Opsi > Tulisan Tangan** membuka panel yang sama seperti Baca Nyaring: pertama **Mesin Tulisan Tangan** (ONNX Runtime), lalu model baris dan daftar kata bahasa yang Anda tulis, masing-masing dengan lisensinya yang ditampilkan sebelum unduhan. **Hapus** menghapus model dan daftar kata. Baris dibaca dengan prioritas rendah pada inti yang ditetapkan di **Berkas > Opsi > Umum > Model AI**, dan model dikeluarkan setelah waktu yang ditetapkan di sana.

**Perlindungan tangan.** Selagi pena menulis, dan sekitar satu detik sesudahnya, sentuhan jari di zona sekitar tulisan (sekitar 30 mm, lebih lebar di sisi tangan penulis dan di bawahnya) diabaikan; kontak yang besar diabaikan selagi pena dipakai; tanpa pena, sentuhan pada jalur sekitar 15 mm di tepi layar diabaikan. Ketukan singkat dan kecil pada tombol atau menu selalu diterima.

**Opsi.** **Berkas > Opsi > Tulisan Tangan** memiliki: **Pengenalan** (bagian pemasangan dengan tombol Instal, bahasa utama, dan bahasa cadangan; bahasa Rusia abu-abu, "belum ada model untuk bahasa ini"); **Menulis** (jeda sebelum satu baris dibaca, berapa lama tinta bertahan, pemudaran, warna dan lebar tinta); **Kisi** (garis bantu, garis tulis dengan garis dasar dan garis tengah, kotak-kotak, titik-titik, atau tanpa; warna, lebar garis, opasitas; ukuran dengan penggeser dari kecil sampai besar, dengan contoh di sampingnya); **Perlindungan tangan** (menyala atau mati, tangan kiri atau kanan, kekuatan); **Pena dan jari**; dan **Mempelajari tangan Anda**. Warna yang Anda biarkan mengikuti tema.

**Mempelajari tangan Anda.** Bila Anda mengoreksi sebuah baris (bacaan lain, atau menulis di atas sebuah kata), gambar tinta dan teks yang dikoreksi disimpan di perangkat ini, di folder data program, dan tidak pernah dikirim ke mana pun. Anda dapat mematikannya dan menghapus baris yang disimpan di halaman yang sama. Selama proyek terenkripsi terbuka, tidak ada yang disimpan (berkasnya akan biasa); halaman menyatakannya.

## 17. Pembaruan dan pengaya

**Bantuan > Pemecahan Masalah dan Pengembangan > Panduan Pengembang: Pembaruan dan Pengaya** menjelaskan, bagi orang yang membangun {{APP}} atau menulis pengaya, bagaimana pembaruan dan pengaya bekerja dan alat apa yang membuatnya; panduan itu juga memuat apa yang diperlukan teman yang paham teknis untuk menyiapkan segalanya (mesin media, plugin format kompilasi, berkas paket, tema, tata letak, dan format skenario, berkas di dalam folder proyek) serta variabel lingkungan yang dibaca program.

{{APP}} adalah satu berkas program (AppImage). Pembaruan dan ekstensi hadir sebagai berkas paket kecil berakhiran **.qbpkg**. Ada tiga jenis, dan semuanya dipasang dengan cara yang sama:

- **Pembaruan program**: versi {{APP}} yang lebih baru.
- **Pengaya**: ekstensi yang dapat Anda nyalakan dan matikan, seperti tema warna, templat proyek, format kompilasi, kamus, alat menulis (bagian 10), pemutaran audio dan video beserta format tambahannya (bagian 6), pengimpor berkas Word lama dan halaman web (bagian 14).

Pembaruan program dan pengaya berada di halaman **Releases** pada halaman proyek (bagian 1), di samping AppImage.
- **Bundel pembaruan**: satu berkas yang memuat pembaruan program dan beberapa pengaya, dipasang bersama.

### Memeriksa pembaruan

Sejak 0.92.0, {{APP}} dapat mencari pembaruan sendiri. Ini satu-satunya hal yang membuat program menghubungi internet dengan sendirinya, dan Anda dapat mematikannya.

- **Bantuan > Pengaya dan Pembaruan > Periksa Pembaruan saat Startup** adalah sakelarnya (menyala pada awalnya). **Bantuan > Pengaya dan Pembaruan > Periksa Pembaruan Sekarang** memeriksa seketika, apa pun kata sakelar.
- **Start pertama** menyampaikannya di halamannya, sebelum apa pun dihubungi: kotak centang *Look for updates when the program starts* (dicentang) dengan catatan bahwa tidak ada yang tentang Anda yang dikirim; satu **OK** mengakhiri halaman. Program yang dijalankan sebelum 0.94.0 dan belum melihat catatan itu menampilkannya sebagai kotak dengan **OK** dan **Matikan**.
- **Apa yang terjadi**: beberapa detik setelah start, paling banyak sekali sehari, program mengambil satu berkas kecil dari halaman proyek (*shelin-boop.github.io/Quillbinder_Collection/updates-1.json*) dan tanda tangan berkas itu. Permintaan menyebutkan program ("{{APP}}") dan tidak ada yang lain: tidak ada versi, tidak ada sistem, tidak ada pengenal, tidak ada cookie. Seperti pada setiap koneksi, server (GitHub) melihat alamat internet Anda dan waktunya. Berkas mencantumkan versi terbaru program dan pengaya proyek; program Anda membandingkannya dengan apa yang terpasang, di komputer Anda. Berkas hanya dianggap bila membawa tanda tangan kunci rilis proyek, dan berkas yang lebih lama daripada yang sudah pernah dilihat diabaikan.
- **Bila ada pembaruan**, sebuah jendela menyebutkannya: program atau pengaya, versi Anda dan yang baru, apa yang baru, dan jenisnya. Pembaruan **fungsional** memiliki tiga tombol: **Unduh dan Instal**, **Abaikan Versi Ini** (tidak ada lagi pengingat untuk versi itu) dan **Tunda**. Tunda menunggu lebih lama setiap kali: 1 hari, 1 minggu, 2 minggu, 4 minggu; lalu ada satu pengingat terakhir, dan setelahnya pengingat untuk versi itu berakhir. Pembaruan **yang disarankan (keamanan)** hanya memiliki dua tombol, **Unduh dan Instal** dan **Tunda**: tidak dapat diabaikan, dan setelah penundaan keempat muncul kembali setiap 4 minggu. Bila kedua jenis ada, satu jendela mencantumkannya dan mengikuti aturan keamanan. Jendela muncul di samping pekerjaan Anda dan tidak mengambil papan ketik.
- **Unduh dan Instal** mengambil berkas yang disebutkan daftar, memeriksa masing-masing terhadap ukuran dan checksum di daftar bertanda tangan, dan memasangnya: pengaya atau pembaruan program seperti yang dilakukan **Bantuan > Pengaya dan Pembaruan > Instal Pembaruan atau Pengaya** (hanya paket dengan tanda tangan proyek). Kemudian program menawarkan untuk memulai ulang.
- **Bila versi baru memerlukan AppImage baru** (pustaka yang berbeda dari yang dimiliki AppImage Anda), {{APP}} mengunduhnya, menaruhnya di tempat AppImage yang Anda jalankan, di mana pun itu dan dengan nama berkas yang sama, menyimpan yang Anda miliki di sampingnya sebagai *…AppImage.before-update*, dan memulai lagi. Hapus berkas yang disimpan itu bila versi baru berfungsi bagi Anda. Jika folder AppImage Anda tidak dapat ditulisi, berkas baru masuk ke folder Downloads Anda dan sebuah pesan menyatakan apa yang harus dilakukan.
- **Di Windows** (0.92.0), versi semacam itu hadir sebagai program setup-nya. {{APP}} mengunduhnya ke folder miliknya sendiri (*data\update-setup*), memeriksanya terhadap daftar bertanda tangan proyek, dan bertanya. Dengan **Tutup dan Perbarui Sekarang**, {{APP}} menutup, dan program setup mengekstrak versi baru ke folder yang Anda pakai, tanpa halaman-halamannya, dan menjalankannya; pengaturan, pengaya, dan segala hal lain di *data* tetap ada. Dengan **Nanti**, tidak ada yang berubah dan pembaruan ditawarkan lagi. Jika Windows tidak menjalankan program setup (tidak memiliki sertifikat penandatanganan kode), sebuah pesan menyebutkan berkasnya, dan Anda juga dapat mengunduh versi terbaru dari halaman proyek.
- **Bila alamat tidak dapat dijangkau**, program mencoba dua kali, berjarak 5 detik, lalu menampilkan teks kecil di bilah footer editor pertama, di tempat teks simpan, yang memudar masuk dan keluar dua kali. Tanpa jendela, dan paling banyak sekali sehari. (**Periksa Pembaruan Sekarang** menjawab dengan sebuah pesan.)
- **Dimatikan**, program tidak menghubungi apa pun, seperti sebelum 0.92.0. Untuk komputer yang disiapkan bagi orang lain, variabel lingkungan `QB_NO_UPDATE_CHECK=1` mencegah pemeriksaan otomatis berjalan, apa pun kata sakelarnya.
- AppImage dari versi sebelum 0.92.0 yang menjalankan pembaruan program yang lebih baru tidak dapat membuat koneksi aman; di sana pemeriksaan menyatakannya ketika Anda memintanya, dan diam bila tidak. Unduh AppImage terbaru satu kali.
- Apa yang diingat program tentang hal ini (sakelar, waktu pemeriksaan terakhir, apa yang Anda abaikan atau tunda) ada di *updates-state.json* di folder data program.

### Memasang paket

1. Pilih **Bantuan > Pengaya dan Pembaruan > Instal Pembaruan atau Pengaya…** dan pilih berkas .qbpkg, atau klik ganda berkas itu di pengelola berkas Anda.
2. {{APP}} menampilkan apa yang dimuat paket dan apa yang baru. Klik **Instal**.
3. Jika paket memuat pembaruan program, {{APP}} meminta untuk memulai ulang. Proyek Anda yang terbuka disimpan dan dibuka kembali.

{{APP}} memeriksa setiap paket sebelum memasangnya: berkas yang rusak, dan paket yang dibuat untuk versi yang berbeda, ditolak. Pembaruan program dijalankan sekali secara tak terlihat sebagai uji; jika tidak berjalan di komputer Anda, pembaruan itu tidak dipasang. Bagian-bagian sebuah bundel dipasang bersama: bila salah satunya tidak dapat dipasang, tidak ada yang dipasang (0.65).

**Tanda tangan** (0.65). Setiap paket di halaman rilis proyek ditandatangani dengan kunci proyek, dan {{APP}} membawa separuh publik kunci itu. Jendela instal menyatakan **Ditandatangani oleh proyek Quillbinder** untuk paket semacam itu: paket itu adalah paket yang diterbitkan proyek, tanpa perubahan.

- Paket yang tanda tangannya tidak cocok (diubah setelah ditandatangani, atau rusak) tidak pernah dipasang.
- **Pembaruan program** dipasang hanya dengan tanda tangan proyek.
- **Pengaya** tanpa tanda tangan itu (yang dibuat Anda atau orang lain) dipasang hanya setelah peringatan yang jelas, dengan **Batal** sebagai bawaan. Pada baris perintah (`--install <file>`), paket tanpa tanda tangan proyek dipasang hanya bila Anda menambahkan `--allow-unsigned`.

**Kunci pengaya** (0.91.3). Pengaya resmi, juga yang ditulis oleh penulis lain, dapat ditandatangani dengan kunci tersendiri alih-alih kunci proyek. Proyek menjamin kunci semacam itu dengan kuncinya sendiri, dalam sertifikat yang datang di dalam paket dan hanya mengizinkan kunci itu menandatangani pengaya, tidak yang lain. Jendela instal kemudian menyebutkan kuncinya: *Ditandatangani dengan kunci pengaya "Quillbinder official add-ons", yang dijamin oleh proyek Quillbinder*.

- **Pembaruan program** yang ditandatangani dengan kunci pengaya ditolak seperti yang tidak bertanda tangan: pembaruan program memerlukan kunci proyek sendiri.
- Kunci pengaya yang tidak dijamin proyek dianggap tanpa tanda tangan: pengaya dipasang hanya setelah peringatan.
- Pengaya yang ditandatangani dengan kunci proyek sendiri, seperti semuanya hingga 0.91.2, tetap dipercaya. Mulai 0.91.3, pengaya milik proyek sendiri juga ditandatangani dengan kunci pengaya.
- Versi hingga 0.91.2 tidak mengenal kunci pengaya dan memperingatkan sebelum memasang pengaya yang ditandatangani dengannya: pasang pembaruan program lebih dulu (ditandatangani dengan kunci proyek sendiri), lalu pengayanya.

**Jendela yang bertanya sebelum pengaya dipasang** (0.94.0) lebih dulu menyatakan, dalam satu atau dua kalimat, apa yang dilakukan pengaya. Baris teknis (pustaka yang dibawanya, mesin ucapan atau grafis, runtime tempat pengaya dibangun) berada di bawah **Rincian**, yang terlipat sampai Anda membukanya.

**Mesin AI dan modelnya** (0.99.0). Fungsi-fungsi AI berjalan di komputer Anda: pengenalan ucapan, membaca dengan nyaring, dan pengenalan tulisan tangan. Mesinnya adalah dua paket tersendiri, **Mesin AI** dan **Mesin Tulisan Tangan**, dan model yang dipakainya adalah unduhan terpisah; tidak ada satu pun yang dipasang bersama {{APP}}, dan tidak ada yang dimuat sampai Anda memakai fungsi seperti itu. Halaman pengaturan setiap fungsi (Baca Nyaring, Tulisan Tangan; Transkripsi Ucapan sejak versi 1.10) menampilkan bagian yang sama untuk ini:

- Keadaan mesin, dengan **Instal Mesin**: mengunduh paket mesin versi Anda dari halaman rilis proyek dan memasangnya seperti pengaya, hanya dengan tanda tangan proyek. **Instal dari Berkas...** mengambil paket yang Anda unduh di tempat lain, untuk komputer tanpa koneksi (paket tanpa tanda tangan proyek hanya setelah peringatan). Mesin yang dipasang selagi mesin lain dipakai berlaku setelah {{APP}} dijalankan lagi. Mesin-mesin itu tercantum di Pengelola Pengaya seperti pengaya dan dapat dimatikan atau dihapus di sana.
- Model fungsi itu, dengan bahasanya, ukuran unduhan, memori yang dibutuhkan, dan lisensinya. **Unduh** mengambil model yang dipilih dari halaman rilis proyek dengan bilah kemajuan (**Batal** menghentikannya; unduhan yang terhenti dilanjutkan dari tempat terhenti), dan memeriksa setiap berkas terhadap checksum-nya sebelum model dianggap terpasang. Model yang memerlukan memori lebih besar daripada yang dimiliki komputer Anda tidak ditawarkan untuk diunduh, dan halaman menyatakan alasannya. **Hapus** menghapus model. Model disimpan di folder data program, di *ai-models*.
- {{APP}} menghubungi internet untuk ini hanya bila Anda menekan salah satu tombol itu, dan tidak menanyakan server apa yang ada: alamatnya adalah alamat rilis proyek.
- Bila sebuah fungsi memungkinkan Anda memilih perangkat: **Otomatis** memilih kartu grafis bila model muat di memori kosongnya (seperti yang dilaporkan drivernya), jika tidak memori dan prosesor komputer; atau kartu grafis pilihan Anda (kartu NVIDIA, AMD, dan Intel melalui Vulkan); atau prosesor. Jumlah inti dan aturan memori adalah yang ada di **Berkas > Opsi > Umum > Model AI**.
- Suara yang dibacakan hanya diputar, tidak pernah disimpan sebagai berkas.

**Memeriksa unduhan sendiri.** Setiap rilis juga memuat *SHA256SUMS.txt* (checksum setiap berkas), tanda tangannya *SHA256SUMS.txt.sig*, dan kunci publik *quillbinder-signing-public.pem*. Di terminal, di folder berisi unduhan:

    openssl pkeyutl -verify -pubin -inkey quillbinder-signing-public.pem -rawin -in SHA256SUMS.txt -sigfile SHA256SUMS.txt.sig
    sha256sum -c --ignore-missing SHA256SUMS.txt

Perintah pertama menjawab *Signature Verified Successfully*, yang kedua *OK* untuk setiap berkas yang Anda unduh. Untuk memastikan berkas kunci itu sendiri milik proyek, bandingkan 32 byte-nya (`openssl pkey -pubin -in quillbinder-signing-public.pem -outform DER | tail -c 32 | od -An -tx1`) dengan kunci di sumber program (*src/core/Signing.cpp*, dan *packaging/quillbinder-signing-public.pem*): keduanya diawali *bc 3c 58 00 08 3f 66 e6*.

### Pengelola Pengaya

**Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya…** menampilkan apa yang terpasang.

- **Versi program**: versi yang tertanam di AppImage Anda dan setiap pembaruan yang terpasang. Yang ditandai *running now* sedang dipakai. Untuk kembali ke versi sebelumnya, pilih dan klik **Pakai Versi Ini**, lalu mulai ulang. Pembaruan terpasang yang tidak lagi Anda perlukan dapat dihapus.
- **Pengaya**: hapus centang pengaya untuk mematikannya tanpa menghapusnya; **Hapus Pengaya** menghapusnya. Tema, templat, dan format kompilasi dari sebuah pengaya muncul di tempat yang biasa: **Tampilan > Tema**, jendela **Proyek Baru**, dan jendela **Kompilasi**.
- Bila pilihan yang dibuat di sini tidak dapat ditulis (disk penuh, folder yang tidak dapat ditulisi), sebuah pesan menyatakannya beserta nama berkasnya (0.92.0). Sebelumnya, jendela menampilkan pilihan seolah sudah dibuat dan dibatalkan pada start berikutnya tanpa kabar. Hal yang sama berlaku untuk pemasangan: pengaya atau versi program yang entrinya tidak dapat ditulis dilaporkan sebagai tidak terpasang, beserta apa yang harus dilakukan. Bila pemasangan gagal di tengah jalan dan versi yang terpasang sebelumnya tidak dapat dikembalikan, pesan menyebutkan folder tempatnya berada dan nama yang harus diberikan kepada folder itu.

### Bila pembaruan tidak mau mulai

Jika pembaruan program gagal dimulai dua kali berturut-turut, {{APP}} mematikannya sendiri dan memulai versi yang tertanam di AppImage, dengan pesan yang memberi tahu Anda. Tidak ada yang di proyek Anda terpengaruh oleh hal ini.

### Bila AppImage baru diperlukan

Pembaruan program berjalan di atas pustaka di dalam AppImage Anda. Dari waktu ke waktu, pembaruan yang lebih besar memerlukan AppImage baru; jendela instal memberi tahu Anda bila sebuah paket tidak cocok dengan milik Anda. Ganti berkas AppImage lama dengan yang baru; pengaturan, pengaya, dan proyek Anda tetap seperti adanya.

0.80.0 adalah versi semacam itu: AppImage-nya membawa Qt baru (build Qt 6.12 milik The Qt Company sendiri sebagai pengganti Qt 6.4.2 milik Ubuntu). Pengaya dengan kode program (*Media Playback*, *Extra Video and Audio Formats*, *Speech Transcription*, alat menulis, dan pengimpor) harus dipasang lagi dalam versinya untuk 0.80.0 (bundel dan **media-all-formats.qbpkg** di halaman Releases membawa semuanya); sampai saat itu, Pengelola Pengaya menandai yang lama sebagai dibangun untuk program yang lebih lama, dan kodenya tidak dimuat. Versi lengkap 0.80.0 membawa setiap pengaya.

0.93.0 adalah satu lagi (Linux): AppImage-nya membawa Qt 6.12.1, yang pustaka bawaannya (zlib, HarfBuzz, PCRE2, libpng, libjpeg-turbo, FreeType, dan lainnya) lebih baru dan memiliki perbaikan keamanan. Pembaruan program tidak dapat membawanya, sehingga pemeriksaan pembaruan menawarkan AppImage baru. Pengaya dengan kode program dari AppImage sebelumnya (yang tercantum di atas) dipasang lagi dalam versinya untuk 0.93.0 dengan cara yang sama, dan pengaya media kini membawa FFmpeg 9.1.

AppImage baru juga membawa versi programnya sendiri (misalnya AppImage 0.20.0). AppImage itu memulai versi tersebut, meskipun Anda telah memasang pembaruan ke AppImage yang lama: pembaruan yang lebih lama dimatikan dan tetap tercantum di **Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya…**, tempat Anda dapat menghapusnya. Jika berkas baru memiliki nama lain atau berada di folder lain daripada yang lama, pilih **Bantuan > Tambahkan ke Menu Aplikasi** satu kali, agar entri menu dan proyek yang diklik ganda memakai berkas yang baru.

---

## 18. Dialek Markdown

Anda tidak perlu mengetahui ini untuk memakai {{APP}}; editor menampilkan teks berformat. Hal ini penting hanya bila Anda membuka berkas teksnya dengan program lain. {{APP}} menulis Markdown dengan aturan berikut:

- **Setiap baris adalah satu paragraf**, seperti di pengolah kata. Baris kosong adalah paragraf kosong.
- Judul: `# ` sampai `###### ` di awal baris.
- Kutipan blok: `> `. Paragraf rata tengah: `->text<-`. Gaya paragraf lainnya: `::Style Name:: text`.
- Gaya karakter (sejak versi 0.4): `<span class="Style Name">text</span>`. Hanya nama gaya yang ditulis ke dalam teks. Bagaimana tampilan sebuah gaya disimpan sekali, di berkas proyek, sehingga gaya yang diubah tidak memerlukan perubahan pada dokumen.
- Daftar: `- ` dan `1. `; dua spasi per tingkat susunan.
- `**bold**`, `*italic*`, `~~strikethrough~~`, `==highlight==`, `<u>underline</u>`, `<sup>superscript</sup>`, `<sub>subscript</sub>`.
- Warna revisi: `<rev1>…</rev1>` sampai `<rev5>…</rev5>`.
- Tautan: `[text](https://example.com)`; tautan ke dokumen di proyek: `[text](qb://ID)`.
- Gambar: `![](images/picture.png){width=300}` (gambar disimpan di folder dokumen). Deskripsi gambar (0.62) adalah teks di dalam tanda kurung siku: `![A brass lamp](images/lamp.png){width=300}`.
- Tabel (0.62): baris `:::table` dengan desain dalam kurung kurawal (JSON), satu baris per baris tabel dengan sel di antara `|` (paragraf sel dipisahkan oleh `<p/>`, sel yang tertutup sel gabungan dibiarkan kosong, `|` di dalam teks ditulis `\|`), dan baris `:::` di akhir.
- Gambar di samping teks atau rata tengah (0.62): `:::figure {"pos":1,"width":168}` (1 kiri, 2 kanan, 3 tengah), baris gambar, baris keterangan, dan `:::`.
- Pemisah baris di dalam paragraf: `<br>`.
- Font yang ditetapkan secara manual (sejak versi 0.3): `<span style="font-family:'DejaVu Sans'; font-size:14pt; color:#aa0000">text</span>`. Deklarasinya adalah CSS biasa, sehingga program Markdown lain yang mengizinkan HTML juga menampilkannya. Entri yang mungkin: `font-family`, `font-size` (pt), `font-weight` (100 sampai 900), `color`, `letter-spacing` (em), `font-variant:small-caps`, `text-transform:uppercase`.
- Pemformatan paragraf yang ditetapkan secara manual (sejak versi 0.3): daftar dalam kurung kurawal di awal baris, sebelum awalan lainnya: `{: align=right left=10mm right=5mm first=0mm before=6pt after=12pt line=1.5 tabs=20mm,50mmR} text`. `line` adalah kelipatan (`1.5`), tinggi pasti (`14pt`), atau minimum (`14pt+`). Perhentian tab diakhiri dengan `R` (kanan), `C` (tengah), atau `D` (titik desimal); tanpa huruf berarti kiri. Hanya entri yang Anda tetapkan yang ditulis.
- Elemen skenario (sejak versi 0.10) adalah gaya paragraf yang dinamai menurut elemennya: `::Script > Scene Heading:: INT. LIGHTHOUSE - NIGHT`. Bagaimana tampilan sebuah elemen disimpan sekali, di berkas proyek, bersama format skenario.
- Pemisah halaman sebelum paragraf (dari skenario dan berkas RTF yang diimpor, sejak versi 0.10): `{: page=before} text`.
- Versi sebelum 0.62 menampilkan tabel dan figur sebagai baris teks (`:::table` ...). Jika Anda kembali ke versi yang lebih lama, jangan edit dokumen dengan tabel atau gambar di samping teks.
- Versi 0.2 menampilkan kedua penanda itu sebagai teks biasa. Jika Anda kembali ke versi 0.2 dengan Pengelola Pengaya, jangan edit dokumen yang memuatnya.
- Versi sebelum 0.4 tidak mengenal gaya Anda sendiri: versi itu menampilkan `<span class=…>` sebagai teks biasa dan memperlakukan paragraf bergaya milik Anda sebagai teks isi. Jika Anda kembali ke versi yang lebih lama, jangan edit dokumen yang memakai gaya Anda sendiri.
- Versi sebelum 0.10 memperlakukan elemen skenario sebagai teks isi, dan menghapus nama elemen dari skenario begitu skenario itu ditampilkan di editor dan proyek disimpan. Jika Anda kembali ke versi yang lebih lama, jangan buka skenario Anda di sana (lihat bagian 20).
- Garis miring terbalik membuat karakter berikutnya menjadi teks biasa: `\*` adalah bintang, bukan huruf miring.
- Catatan mengikuti konvensi CriticMarkup:
  - `{>>inline annotation<<}`
  - `^[inline footnote]`
  - `{==text==}{>>linked comment<<}`
  - `{==text==}^[linked footnote]`
  - Komentar dapat diawali dengan warna: `{>>@#ff8800 text<<}`.

**Berkas > Konversi Markdown ⇄ Berkas RTF…** membaca dialek ini maupun Markdown standar (tempat paragraf dipisahkan baris kosong), dan saat menulis Markdown, perintah ini bertanya mana yang Anda inginkan dari keduanya.

---

## 19. Pintasan papan ketik

### Mengubah pintasan

**Berkas > Opsi > Papan Ketik** (atau **Bantuan > Pintasan Papan Ketik**) mencantumkan setiap perintah, dikelompokkan menurut menu, dengan pintasannya dan pintasan standar bawaan program. Gaya proyek yang terbuka dan tata letak juga dicantumkan, sehingga sebuah gaya atau tata letak dapat memiliki pintasan sendiri.

1. Temukan perintahnya: gulir, atau ketik sebagian namanya atau pintasan seperti "Ctrl+K" di kolom pencarian.
2. Klik perintahnya, klik kolom pintasan di bawah daftar, dan tekan tombol baru.
3. Jika perintah lain memiliki pintasan itu, Anda ditanya apakah akan mengambilnya dari sana. Perintah lain itu kemudian tidak memiliki pintasan sampai Anda memberinya yang baru.

**Tanpa Pintasan** menghapus pintasan perintah yang dipilih, **Standar** mengembalikan yang bawaannya, dan **Semua ke Standar** membatalkan semua perubahan Anda. Perubahan Anda ditampilkan tebal. Pintasan memerlukan Ctrl, Alt, atau Meta, atau harus berupa tombol fungsi, karena huruf biasa diperlukan untuk menulis. **Ekspor** dan **Impor** menyimpan kumpulan pintasan Anda sebagai berkas (*.qbkeys*) dan memuatnya, misalnya untuk dibawa ke komputer lain.

Pintasan gaya mengikuti nama gaya: "Block Quote" memiliki pintasannya di setiap proyek yang memiliki gaya dengan nama itu.

### Pintasan dan tata letak papan ketik

Beberapa pintasan standar adalah tombol yang memerlukan **AltGr** pada papan ketik Jerman: **Ctrl+{**, **Ctrl+|** dan **Ctrl+}** (rata kiri, tengah, kanan), **Ctrl+[** dan **Ctrl+]** (mundur dan maju), **Ctrl+\\** (daftar elemen skenario), dan **Ctrl+=** (subskrip). Sejak 0.94.0, program melihat tata letak papan ketik sistem **pada start yang paling pertama** (bila belum ada pengaturan pintasan dan start sebelumnya) dan menyimpan jawabannya. Dengan **tata letak AS**, atau tata letak apa pun selain Jerman, kumpulan yang biasa di atas tetap berlaku. Dengan **tata letak Jerman (atau Austria)**, perintah-perintah ini mendapat tombol yang tidak memerlukan AltGr (kumpulan yang sama berlaku untuk **tata letak Prancis, Belgia, Spanyol, Italia, Portugis, Turki, atau Belanda**, yang tanda kurung siku dan kurung kurawalnya juga memerlukan AltGr; tata letak programmer Polandia, yang huruf AltGr-nya ć dan ń bertabrakan dengan Ctrl+Alt+C dan Ctrl+Alt+N, hanya memindahkan empat perintah yang disebutkan di bawah tabel):

| Perintah | Kumpulan Jerman |
|---------|------------|
| Rata kiri / tengah / kanan (juga di peta pikiran) | Alt+Shift+L / Alt+Shift+E / Alt+Shift+R |
| Rata kiri-kanan (peta pikiran) | Alt+Shift+J |
| Mundur / maju | Alt+Shift+B / Alt+Shift+F |
| Daftar elemen skenario | Ctrl+Shift+O |
| Subskrip | Ctrl+Shift+D |
| Bawa ke depan / kirim ke belakang (peta pikiran) | Ctrl+Shift+PageUp / Ctrl+Shift+PageDown |

Tata letak yang memakai AltGr untuk huruf (Polandia, Brasil, dan yang lain di atas) juga memindahkan empat perintah dari Ctrl+Alt+huruf, yang dibaca Windows sebagai AltGr: salin pemformatan Ctrl+Shift+Y, tempel pemformatan Ctrl+Shift+U, buka catatan dokumen Ctrl+Shift+X, dan (tidak pada tata letak Polandia) tampilkan penggaris Ctrl+Shift+W. Papan ketik Rusia dan Indonesia mempertahankan kumpulan yang biasa.

Menu menampilkan setiap tombol seperti yang dinamai tata letaknya. Program yang sudah dipakai sebelum 0.94.0 mempertahankan pintasannya seperti semula (tidak ada yang berubah dengan sendirinya), begitu pula setiap pintasan yang Anda tetapkan sendiri di **Berkas > Opsi > Papan Ketik**: pintasan yang tersimpan tidak pernah diganti. Sejak 0.95 tombol **Pakai Set untuk Tata Letak Papan Ketik Saya…** di bagian bawah **Berkas > Opsi > Papan Ketik** menerapkan set yang cocok dengan tata letak papan ketik yang sedang dipakai (untuk papan ketik Jerman, tombol-tombol di atas). Tombol itu lebih dulu mencantumkan pintasan standar mana yang berubah; pintasan yang Anda tetapkan sendiri tetap ada. Set itu menjadi milik profil Anda. Bila sistem tidak menyebutkan tata letaknya, kumpulan yang biasa dipakai. (Linux: pengaturan papan ketik desktop atau server X; Windows: bahasa tata letak papan ketik; Android: bahasa papan ketik.)

### Pintasan standar

| Tindakan | Pintasan |
|--------|----------|
| Proyek baru / buka proyek | Ctrl+Shift+N / Ctrl+O |
| Simpan / tutup proyek / keluar | Ctrl+S / Ctrl+W / Ctrl+Q |
| Dokumen baru / folder baru | Ctrl+N / Alt+Shift+N |
| Ganti nama / duplikat | F2 / Ctrl+D |
| Pindahkan item ke atas / bawah / keluar / masuk | Ctrl+Alt+Up / Down / Left / Right |
| Pisahkan pada pilihan / dengan pilihan sebagai judul | Ctrl+K / Ctrl+Shift+K |
| Gabungkan dokumen | Ctrl+Shift+M |
| Teks Gabungan / papan gabus / kerangka | Ctrl+1 / Ctrl+2 / Ctrl+3 |
| Tampilkan atau sembunyikan binder / inspektor | Ctrl+Alt+B / Ctrl+Alt+I |
| Mode menulis / layar penuh | F11 / Ctrl+Shift+F11 |
| Fokus ke binder / fokus ke editor / editor lain | Alt+1 / Alt+2 / F6 |
| Mundur / maju | Ctrl+[ / Ctrl+] (tata letak Jerman: Alt+Shift+B / Alt+Shift+F) |
| Dokumen sebelumnya / berikutnya | Alt+Shift+Up / Alt+Shift+Down |
| Tunjukkan di binder | Ctrl+Shift+R |
| Cari / ganti / berikutnya / sebelumnya | Ctrl+F / Ctrl+H / F3 / Shift+F3 |
| Pencarian proyek | Ctrl+Shift+F |
| Pakai pilihan untuk pencarian | Ctrl+E |
| Cari menurut pemformatan / berikutnya / sebelumnya | Ctrl+F3 / Ctrl+G / Ctrl+Shift+G |
| Catatan sementara (pengaya) | Alt+Shift+Return; dari program mana pun: lihat bagian 10 |
| Audio dan video (pengaya): putar / jeda | Ctrl+Return |
| Audio dan video: lompat mundur / lompat maju | Ctrl+Alt+, / Ctrl+Alt+. |
| Audio dan video: sisipkan stempel waktu | Ctrl+Shift+Return |
| Saring papan gabus atau kerangka | Ctrl+F (saat salah satunya menerima papan ketik) |
| Tautan ke dokumen | Ctrl+L |
| Komentar / catatan kaki | Shift+F2 / Ctrl+Shift+F2 |
| Anotasi sebaris / catatan kaki sebaris | Ctrl+Shift+A / Ctrl+Alt+F |
| Buat snapshot / dengan judul | Ctrl+5 / Ctrl+Shift+5 |
| Target proyek / kata kunci proyek | Ctrl+Shift+T / Ctrl+Alt+K |
| Impor berkas / kompilasi | Ctrl+Shift+I / Ctrl+Shift+E |
| Cetak dokumen saat ini | Ctrl+P |
| Jendela font / jendela paragraf | Ctrl+T / Ctrl+Alt+P |
| Ukuran font lebih besar / lebih kecil | Ctrl+> / Ctrl+< |
| Salin pemformatan / tempel pemformatan | Ctrl+Alt+C / Ctrl+Alt+V |
| Tempel dengan pemformatan / tempel dan samakan gaya | Ctrl+Alt+Shift+V / Ctrl+Shift+V |
| Tanpa Gaya / Heading 1 / Heading 2 / Heading 3 | Ctrl+Alt+0 / Ctrl+Alt+1 / Ctrl+Alt+2 / Ctrl+Alt+3 |
| Enam gaya berikutnya dalam daftar gaya (Format > Gaya > Sesuaikan Daftar Gaya) | Ctrl+Alt+4 ... Ctrl+Alt+9 |
| Buka ke sinopsis / buka ke catatan dokumen (Navigasi > Inspeksi) | Ctrl+Alt+Y / Ctrl+Alt+N |
| Cadangkan sekarang | Ctrl+Shift+B |
| Mode skenario / daftar elemen skenario | Ctrl+8 / Ctrl+\\ (tata letak Jerman: Ctrl+8 / Ctrl+Shift+O) |
| Dalam skenario: elemen berikutnya / ubah elemen | Enter / Tab (lihat bagian 4, Penulisan Skenario) |
| Perbesar / perkecil / atur ulang zoom | Ctrl++ / Ctrl+- / Ctrl+0 |
| Zoom area di bawah tetikus | Ctrl+roda tetikus |
| Tampilkan atau sembunyikan penggaris | Ctrl+Alt+R |
| Pengguliran mesin ketik | Meta+Ctrl+T |
| Opsi | Ctrl+, |
| Panduan ini | F1 |

---

## 20. Pemecahan masalah

### Laporan crash dan informasi sistem

Jika {{APP}} berakhir karena kesalahan, program menulis **laporan crash** di komputer Anda sebelum menutup. Jika jendelanya tidak merespons selama 30 detik (macet; waktunya diatur di **Berkas > Opsi > Ekstra > Laporan Crash**), program menulis **laporan pembekuan** yang menunjukkan di mana program tersangkut; jika pulih, laporan menyebutkan setelah berapa lama. Tidak ada yang dikirim ke mana pun. Saat Anda memulai {{APP}} berikutnya setelah crash, atau setelah Anda harus mengakhiri {{APP}} yang membeku, sebuah pesan memberi tahu apa yang terjadi. Jika program berakhir dalam satu menit setelah dimulai, pesan menawarkan **Mulai Tanpa Proyek Terakhir**, kalau-kalau proyek itu penyebabnya.

- **Bantuan > Pemecahan Masalah dan Pengembangan > Laporan Crash…** mencantumkan laporan crash dan pembekuan beserta tanggal dan jenis kesalahannya ("freeze (recovered)" bila {{APP}} terus bekerja). **Salin ke Papan Klip** menyalin yang dipilih, sehingga Anda dapat menempelkannya ke pesan untuk siapa pun yang mengurus {{APP}} bagi Anda; **Buka Folder** menampilkan berkasnya; **Hapus** dan **Hapus Semua…** menghapusnya. 20 laporan terbaru disimpan (**Berkas > Opsi > Ekstra > Laporan Crash** mengubah jumlahnya).
- **Berkas > Opsi > Umum > Peringatan > Tampilkan Konsol Log Internal** menampilkan pesan program dan Qt sejak dimulai (1000 yang terakhir), dengan pesan baru saat datang; berguna bila sesuatu tidak berfungsi dan tidak ada pesan yang menyatakan alasannya.
- **Bantuan > Pemecahan Masalah dan Pengembangan > Informasi Sistem** menampilkan potret program dan komputer yang sama dengan yang dibawa laporan. Dimulai dengan **Hal yang perlu diperhatikan**: apa yang dapat menimbulkan masalah dalam waktu dekat, seperti drive yang hampir penuh atau hanya-baca, proyek di folder jaringan, drive cadangan dengan ruang kosong lebih sedikit daripada yang dibutuhkan proyek, memori kosong yang sedikit, waktu simpan otomatis yang lama, cadangan otomatis yang dimatikan, atau laporan crash dan pembekuan terbaru. Kemudian menyusul versi program, runtime (kumpulan pustaka di dalam AppImage, dengan hari rilis Qt-nya dan seberapa lama: runtime yang lama berarti AppImage yang lebih baru mungkin membawa perbaikan keamanan) serta pembaruan dan pengaya yang terpasang, sistem operasi, desktop, layar (sejak 0.93.1 dengan area yang dapat dipakai jendela pada masing-masing), perangkat masukan (papan ketik, tetikus, layar sentuh, pena) dan apakah papan ketik layar ditampilkan (0.93.1), prosesor dan memori, folder beserta ruang kosongnya, pengaturan untuk penyimpanan dan cadangan, dan proyek yang terbuka. Gunakan **Salin ke Papan Klip** atau **Simpan Sebagai…** ketika Anda melaporkan masalah yang bukan crash.

Laporan menyebutkan berkas, folder, dan dokumen, serta perintah dan tombol yang dipakai sesaat sebelum kesalahan (huruf yang diketik hanya dihitung, tidak pernah direkam). Laporan tidak memuat teks dokumen Anda. Folder home Anda ditulis sebagai **~**.

Jika {{APP}} dihentikan dari luar atau komputer dimatikan tanpa menutupnya, tidak ada laporan, tetapi start berikutnya juga memberi tahu Anda. Dalam kedua kasus, pekerjaan Anda tersimpan hingga simpan otomatis terakhir, beberapa detik sebelum akhir; lihat snapshot dokumen atau cadangan terbaru bila ada yang hilang (bagian 11 dan 15).

**"Berkas proyek tidak dapat dibaca."** Jika berkas proyek (*JudulAnda*{{FILE}}) rusak, misalnya karena komputer crash saat berkas disimpan, {{APP}} menawarkan salinan yang disimpannya (dari terakhir kali proyek dibuka): **Pakai Salinan**. Berkas yang rusak disimpan di sampingnya (*… .damaged-<tanggal>*). Perubahan pada binder sejak saat itu (judul, urutan, label) hilang; semua teks ada, dan dokumen yang dibuat sejak saat itu ditaruh di folder **Recovered Documents** di Riset, dinamai menurut baris pertamanya.

**"Berkas proyek memuat karakter yang tidak diizinkan di dalamnya."** Versi sebelum 0.20 dapat menyimpan karakter kontrol tak terlihat (ditempel dari program lain ke judul atau sinopsis), yang setelahnya membuat proyek tidak lagi terbuka. {{APP}} menghapusnya saat membuka proyek dan memberi tahu Anda; tidak ada hal lain yang diubah. Sejak 0.20, karakter itu tidak pernah disimpan.

**Dokumen ditampilkan kosong, dengan catatan "Berkas dokumen ini tidak dapat dibaca".** Berkas teksnya ada tetapi tidak dapat dibaca (izinnya diubah, atau drive tempatnya berada hilang). {{APP}} menampilkannya kosong dan tidak membiarkan Anda mengeditnya, agar berkas itu tidak pernah tertimpa. Periksa izin berkas atau drive-nya, lalu tutup dan buka proyek lagi.

**Menekan Page Down atau Page Up menutup program.** Ini adalah kesalahan pada versi sebelum 0.19; perbarui ke 0.19 atau lebih baru. Sampai saat itu, gulir dengan tetikus atau bilah gulir.

**Program tidak mulai saat saya mengklik gandanya.** Klik kanan berkas > Properties > Permissions dan centang "Is executable" (di Dolphin: "Allow executing file as program"). Lalu klik ganda lagi.

**Dialog berkas tampak berbeda dari program KDE lainnya.** Nyalakan **Pakai dialog berkas desktop** di **Berkas > Opsi > Umum** dan mulai ulang {{APP}}.

**Ikon bilah alat sulit dilihat pada tema gelap atau terang.** {{APP}} memilih set ikon desktop Anda yang terang atau gelap agar sesuai dengan tema. Jika desktop tidak memiliki set yang cocok, program memakai ikonnya sendiri. Jika ikon tertentu masih tidak cocok, pilih **Default (Follow the Desktop)** di **Tampilan > Tema** dan atur warnanya di pengaturan desktop sendiri.

**Saya mengubah warna dan ingin warna tema kembali.** **Berkas > Opsi > Penampilan > Atur Ulang Warna dan Font ke Tema**, atau tombol **↺** di samping satu warna atau font.

**Pembaruan program tidak mulai.** {{APP}} mematikan pembaruan yang gagal dengan sendirinya (bagian 17). Anda dapat mencoba memasangnya lagi, atau terus memakai versi bawaan.

**Format kompilasi hilang.** Periksa **Berkas > Opsi > Plugin**: di sana tercantum program yang diperlukan format itu. Pasang di Discover dan mulai ulang {{APP}}.

**Pemeriksa ejaan tidak mengenal bahasa saya.** Untuk bahasa Spanyol, Prancis, Portugis Brasil, Italia, Rusia, Indonesia, Turki, Polandia, dan Belanda, pasang pengaya "kamus ejaan <bahasa>" (**Bantuan > Pengaya dan Pembaruan > Instal Pembaruan atau Pengaya**; halaman unduhan mencantumkannya), mulai ulang {{APP}}, dan pilih bahasanya di **Berkas > Opsi > Koreksi** (halaman Ejaan). Untuk bahasa lain, pasang paket kamus *hunspell* yang sesuai di Discover.

**Proyek "tampaknya terbuka di jendela lain atau di komputer lain."** Hal ini terjadi bila proyek berada di folder tersinkron yang terbuka di tempat lain, atau setelah crash di komputer lain. Jika Anda yakin proyek tidak terbuka di tempat lain, pilih **Ya**.

**Alt+seret tidak membuka pemegang salinan.** Beberapa desktop memakai Alt+seret untuk keperluannya sendiri, seperti memindahkan jendela. Pakai **Navigasi > Buka > Di Pemegang Salinan** atau menu klik kanan dokumen sebagai gantinya, atau ubah tombolnya di pengaturan jendela desktop.

**Setelah kembali ke versi program yang lebih lama, skenario menjadi teks biasa tanpa tata letaknya.** Versi sebelum 0.10 tidak mengenal elemen skenario dan menyimpan skenario yang telah ditampilkannya sebagai teks biasa. Beralih lagi ke versi saat ini (**Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya**). Semua teks ada; untuk mengembalikan tata letaknya, pulihkan dokumen dari snapshot atau cadangan yang dibuat sebelumnya (bagian 11 dan 15), atau berikan lagi elemennya kepada paragraf dengan menu pop-up di footer.

**Pintasan catatan sementara tidak melakukan apa-apa di program lain.** Pintasan yang ditetapkan di **Berkas > Opsi > Papan Ketik** hanya bekerja di dalam {{APP}}. Untuk program lain, desktop harus menjalankan perintahnya (bagian 10, *Dari program mana pun*). Jika Anda memindahkan AppImage, masukkan perintahnya lagi (perintah itu menyebutkan lokasi AppImage).

**Kamera catatan sementara menyatakan tidak dapat membaca layar.** Di Wayland hanya program tangkapan layar milik desktop yang boleh melakukannya. Pasang *Spectacle* (`sudo apt install kde-spectacle`) dan coba lagi.

**Edit > Alat Menulis menyatakan "No writing tools installed".** Alat itu adalah pengaya: pasang paket pembaruan 0.11 atau nyalakan di **Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya**.

**Berkas audio atau video hanya menawarkan "Open with Default Application".** Memutar di dalam {{APP}} memerlukan pengaya *Media Playback* (dalam paket pembaruan 0.12); pasang, atau nyalakan di **Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya**. Baris di bawah tombol menyatakan apa yang hilang.

**Video diputar tetapi tidak ada suara, atau berkas audio menyatakan "No sound output was found".** {{APP}} memutar suara melalui server suara desktop (PipeWire atau PulseAudio). Periksa bahwa suara berfungsi di program lain dan bahwa keluaran yang benar dipilih di pengaturan audio desktop; {{APP}} mengikuti perubahan keluaran seketika.

**"This file cannot be played here: it uses H.264 and AAC ..."** Berkas itu dalam format berlisensi paten, yang tidak disertakan *Media Playback*. Pasang pengaya **Extra Video and Audio Formats**, atau paket **media-all-formats.qbpkg**, yang membawanya bersama *Media Playback* (keduanya di halaman Releases pada halaman proyek, bagian 1), dan mulai ulang {{APP}}.

**Berkas tidak diputar, atau diputar tanpa gambar.** Beberapa jenis bukan bagian dari penguraian bawaan (video AV1, WMA Pro, WMV 9). Buka berkas dengan program lain (klik kanan pemutar: **Open with Default Application**), atau konversi ke format lain.

**Berkas .doc ditambahkan sebagai berkas, bukan sebagai teks.** Pengaya **Word 97–2003 Import** tidak terpasang atau dimatikan (**Bantuan > Pengaya dan Pembaruan > Pengelola Pengaya**); setelah memasangnya, mulai ulang {{APP}}. Pesan setelah impor menyebutkan apa yang dihilangkan pengaya. Berkas Word 6.0 dan 95 memerlukan LibreOffice (`sudo apt install libreoffice-writer`).

**Halaman web masuk kosong atau dengan bagian yang salah.** Hapus centang **Hanya artikel** di jendela Halaman Web untuk mengambil seluruh halaman dan hapus apa yang tidak Anda perlukan. Halaman yang memerlukan login atau dibangun sepenuhnya oleh skrip tidak dapat dibaca tanpa browser: buka halaman di browser Anda, pilih teksnya, dan seret ke binder (menjadi dokumen baru). "Secure web pages cannot be fetched" berarti OpenSSL 3 tidak ada (`sudo apt install libssl3`, bagian dari setiap Ubuntu) atau pengaya rusak (pasang lagi).

**Detik pertama berkas senyap atau waktu diam di awal.** Keluaran suara memerlukan waktu sebentar untuk mulai; pada beberapa server suara lebih dari satu detik. Putar mundur sedikit (Ctrl+Alt+,) bila Anda melewatkan awalnya.

**Halaman di tampilan halaman tidak cocok dengan naskah cetak saya.** Memang tidak dimaksudkan demikian: tampilan halaman menampilkan teks Anda pada lembaran selagi Anda menulis. Format kompilasi menentukan halaman berkas jadi (bagian 13).

**"Scrivener has this project open."** Scrivener meninggalkan berkas kunci di *.scriv* selama proyek terbuka di sana, dan setelah crash. Tutup proyek di Scrivener (di setiap komputer) dan buka lagi di sini; jika Scrivener tidak berjalan di mana pun, pilih **Ya**.

**Proyek dalam format Scrivener tidak menampilkan perubahan terbaru saya dari Scrivener.** {{APP}} membaca *.scriv* lagi hanya saat proyek dibuka. Tutup di sini (**Berkas > Tutup Proyek**), biarkan Scrivener atau layanan sinkronisasi selesai, lalu buka lagi.

**Menu Sisipkan, Navigasi, Proyek, Dokumen, dan Format berwarna abu-abu.** Tidak ada proyek yang terbuka. Buka atau buat satu (menu Berkas).

**Ke mana melaporkan masalah.** **Bantuan > Laporkan Masalah** membuka halaman isu proyek (https://github.com/Shelin-boop/Quillbinder_Collection/issues/new/choose) di browser Anda, dengan formulir untuk masalah dan satu untuk permintaan; Anda memerlukan akun GitHub, dan Anda dapat menulis dalam bahasa Inggris atau Jerman. Isu bersifat publik: jangan lampirkan naskah atau proyek Anda; **Bantuan > Pemecahan Masalah dan Pengembangan > Informasi Sistem** (**Salin ke Papan Klip**) memuat apa yang diperlukan laporan tentang program dan komputer. **Masalah keamanan** tidak dilaporkan di sana tetapi secara pribadi, seperti dijelaskan di *SECURITY.md* proyek (tab Security repositori, **Report a vulnerability**).

**Daftar isi berkas Word tidak memiliki nomor halaman.** Nomor halaman diisi oleh pengolah kata ketika memperbarui bidang; perbarui di sana.

**Ada yang salah dengan sebuah dokumen.** Lihat tab snapshot inspektor, atau pulihkan cadangan (bagian 15).

## Kredit dan lisensi

- Pengembang Utama: Claude.AI w. Opus 5.5 Extra

{{APP}} adalah perangkat lunak bebas di bawah Apache License 2.0: siapa pun boleh memakai, menyalin, mengubah, dan meneruskannya, dengan atau tanpa biaya, dengan tetap menyertakan lisensi dan berkas NOTICE, yang menyebutkan proyek dan sumbernya; tanpa jaminan. Versi hingga 0.70.0 diterbitkan di bawah lisensi Zero-Clause BSD (0BSD). Jendela Tentang (di menu Bantuan) menampilkan versi dan lisensinya.

{{APP}} memakai bagian dari proyek lain (Qt, Hunspell, ICU, ikon Breeze, dan pustaka lain di dalam AppImage), masing-masing di bawah lisensinya sendiri. **Bantuan > Lisensi Pihak Ketiga** (juga tombol di jendela Tentang) mencantumkannya beserta teks lisensi lengkapnya; teks yang sama ada di dalam AppImage, di *usr/share/doc/quillbinder/licenses*. Kode sumber masing-masing, persis pada versi yang dipakai, ada di rilis **Third-party source code** pada halaman proyek (https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources); jendela menyebutkan berkas untuk setiap pustaka, dan catatan lisensi pengaya melakukan hal yang sama untuk milik mereka.

Scrivener dan Scapple adalah nama produk Literature & Latte. {{APP}} adalah program independen, tidak dibuat oleh atau terhubung dengan Literature & Latte; program ini membaca dan menulis format berkas mereka agar proyek Anda dapat berpindah di antara program-program tersebut.

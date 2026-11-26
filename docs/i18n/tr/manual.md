# {{APP}} Kullanım Kılavuzu

Bu kılavuz makine tarafından çevrilmiştir ve henüz Türkçe bilen biri tarafından denetlenmemiştir. Anlaşılmayan yerlerde İngilizce kılavuz esas alınır.

{{APP}}, uzun metinler için bir yazı stüdyosudur: romanlar, senaryolar, oyunlar, çizgi roman metinleri, tezler ve kurgu dışı kitaplar. Küçük parçalar halinde (sahneler, bölümler, notlar) yazarsınız, bunları özgürce düzenlersiniz ve sonunda tek bir müsvedde olarak *derlersiniz*.

Bu kılavuz {{VERSION}} sürümünü kapsar. Açmak için istediğiniz zaman **F1** tuşuna basın; baktığınız şeye uyan bölümde açılır: belge, mantar pano, anahat veya Birleşik Metin, binder, klavyenin bulunduğu denetçi sekmesi ya da açık olan iletişim penceresi (kendi bölümü olmayan bir iletişim penceresi başlangıçta açılır). Pencerenin solunda bir **içindekiler listesi** (bir başlığa tıklamak oraya atlar) ve metnin üstünde bir **arama alanı** vardır: oraya gitmek için **Ctrl+F** tuşlarına basın, bir sözcük yazın; **Return** sonraki eşleşmeyi, **Shift+Return** önceki eşleşmeyi gösterir (**Sonraki** ve **Önceki** düğmeleri de aynısını yapar); alanın yanındaki satır, kaç eşleşmeden hangisinin gösterildiğini söyler ve her eşleşme metinde işaretlenir. {{APP}}'i geliştirenlere yönelik konular (eklentiler, paketler, proje klasörünün ayrıntıları, ortam değişkenleri) geliştirici kılavuzundadır: **Yardım > Sorun Giderme ve Geliştirme > Geliştirici Kılavuzu: Güncellemeler ve Eklentiler**.

---

## 1. İlk adımlar

### Programı edinmek

{{APP}}, proje sayfasının **Releases** sayfasından indirilir: https://github.com/Shelin-boop/Quillbinder_Collection/releases. Her sürümde program tek bir dosya olarak bulunur (*{{APP}}-&lt;version&gt;-x86_64.AppImage*); ayrıca {{APP}}'in AppImage dosyasına zaten sahip olanlar için bir program güncellemesi (bölüm 17) ve eklentiler (*.qbpkg* dosyaları, bölüm 17) vardır. AppImage dosyasını çalıştırılabilir yapın (sağ tık > **Properties** > **Permissions** > *Is executable* ya da bir terminalde `chmod +x`) ve çift tıklayın; **Yardım > Uygulama Menüsüne Ekle** programı masaüstünüzün menüsüne koyar. Her sürümün *SHA256SUMS.txt* dosyası, dosyalarının sağlama toplamlarını listeler.

**{{APP}}, {{LABEL}} bir yazılımdır.** Projelerinizin yedeklerini (bölüm 15) ve müsveddelerinizin bir kopyasını programın dışında tutun.

### Bir proje başlatmak

{{APP}} projesiz başladığında **başlangıç penceresi** açılır:

- **Yeni Proje**: bir kategori ve bir şablon seçin (örneğin Blank, Novel, Novel with Parts veya Short Story), bir ad yazın, kaydedilecek yeri seçin ve **Oluştur** düğmesine tıklayın. **Format**, projenin nasıl saklanacağını belirler: {{APP}} projesi olarak (standart) ya da Scrivener 3 projesi (*.scriv*) olarak; böylece onu Scrivener'da da açabilirsiniz (bölüm 14, *Scrivener biçiminde çalışmak*).
- **Son Projeler**: daha önce üzerinde çalıştığınız bir projeye çift tıklayın.
- **Diğerini Aç…**: diskinizde bir proje bulun.
- **Scrivener Projesini İçe Aktar…**: bir Scrivener projesinden {{APP}} projesi yapar (bkz. bölüm 14). Scrivener projesinin kendisi üzerinde çalışmaya devam etmek için bunun yerine *.scrivx* dosyasını **Diğerini Aç…** ile açın.

{{APP}} başlarken son projeyi yeniden açar (bunu **Dosya > Seçenekler > Genel** altından kapatabilirsiniz).

**İlk başlatma**, her şeyi önceden doldurulmuş tek bir sayfa gösterir (**Tamam** onu olduğu gibi kabul eder): yedekler (projenin kapatılırken yedeklenip yedeklenmeyeceği, kaç yedek tutulacağı, başlangıçta 25, ve nerede), bir Linux AppImage'ından başlatıldıysa {{APP}}'in uygulama menüsüne eklenip eklenmeyeceği sorusu (ve AppImage'ın *~/Applications* klasörüne taşınıp taşınmayacağı) ve güncelleme denetimiyle ilgili not ile anahtarı. **Tamam**'e bastığınızda karşılama projesi açılır; ondan önce boş bir program penceresi görünmez. Bunların hepsi sonradan **Dosya > Seçenekler** içinde değiştirilebilir; orada, **Genel > Başlangıç** altında, hangi ayarların önce geleceğinin, yani projenin kendi ayarlarının mı yoksa her zaman programın ayarlarının mı geçerli olacağının seçimi de bulunur (bölüm 16, *Tek bir proje için ayarlar*).

### Karşılama projesi ve Hızlı Başlangıç Kılavuzu

{{APP}} ilk kez başladığında **karşılama projesini** açar: {{APP}}'in olağan olanın ötesinde neler sunduğunu, her biri ayrı bir belgede gösteren kısa bir proje (simge çubuğu, pencerenin her bölümü için bir yakınlaştırma, zihin haritaları, nesne sayfaları, bütün bir klasörün yorumları, Karşılaştır ve Aktar, Scrivener projeleri, temalar, güncellemeler ve eklentiler ve metninizi koruyan şeyler). Binder'ın en üstündeki **Karşılama** belgesiyle başlayın; bağlantıları turun her durağına götürür. Karşılama projesinde yaptığınız hiçbir şey kendi projelerinize zarar veremez. (Komut satırında verilen bir proje ya da çift tıklanan bir proje dosyası bunun yerine açılır; karşılama projesi o zaman bir sonraki başlatmada gelir.)

Karşılama projesi, *Documents* klasörünüze "Welcome to {{APP}}" adıyla kaydedilir. **Yardım > Karşılama Projesi…** onu bıraktığınız gibi yeniden açar ya da baştan başlamak için yeni bir kopya oluşturur (**Yeni Bir Kopya Oluştur**; eski kopya masaüstünün çöp kutusuna gider). İstediğiniz zaman silebilirsiniz; komut yenisini oluşturur.

**Yardım > Hızlı Başlangıç Kılavuzu**, önemli ayarların nerede olduğunu (temalar ve koyu mod, renkler ve yazı tipleri, biçimlendirme, yakınlaştırma, araç çubukları, kısayollar, yedekler ...) tek sayfada gösterir ve {{APP}}'in neler yapabildiğini özetler.

### Kaydetme

Elle kaydetmeniz hiç gerekmez. {{APP}}, yazmayı bıraktıktan birkaç saniye sonra, başka bir programa geçtiğinizde ve projeyi kapattığınızda otomatik olarak kaydeder. Yazmaya devam ederseniz, 5 saniye geçtikten sonraki ilk kısa aralıkta (sözcükler arasında bir soluk) kaydeder; hiç ara olmasa bile en geç 10 saniye sonra kaydeder. Emin olmak isterseniz **Ctrl+S** hemen kaydeder.

Her dosya önce geçici bir adla yazılır ve eski dosyanın yerini ancak bütünüyle yazıldığında ve bilgisayar onu diske işlediğinde alır. Kaydederken disk dolarsa ya da sürücü arızalanırsa eski dosya olduğu gibi kalır ve değişiklikleriniz kaydetme yeniden çalışana kadar pencerede durur.

**Kaydetme durumu her zaman ekrandadır** (0.70). İlk düzenleyicinin alt çubuğu (klasör toplamı ile sözcük sayısı arasındaki boş kısım, diğer alt çubuklarda ise sol taraf) **Kaydedildi**, **Henüz kaydedilmedi** (yazdıktan birkaç saniye sonra) ya da kırmızı bir zeminde **Kaydedilmedi: son kaydetme 14:03'te başarısız oldu** der; kaydetme başarısız olduğu sürece pencerenin başlığında da bir işaret bulunur. Başarısız bir kaydetme ayrıca kaydedilmeyen belgeleri adlarıyla belirten ve **Metinleri Başka Yere Kaydet…** seçeneğini sunan bir ileti gösterir; klavyeyi asla almaz, yani yazdığınız bir sonraki tuş yine metninize girer. Program kendiliğinden yeniden dener (önce 15 saniyede bir, sonra dakikada bir), başarısızlık sürerse ileti beş dakika sonra yeniden gelir ve kırmızı işaret, bir kaydetme başarılı olana kadar kalır. Zihin haritaları da aynı şekilde kaydedilir ve denetlenir: kaydedilemeyen bir pano pencerede tutulur, proje kapatılırken kaydedilmemiş sayılır ve acil durum kopyasına girer.

**Kısa iletiler** (0.93.0). Pencerenin altında bir durum çubuğu satırı yoktur. Eskiden orada görünen iletiler ("Saved again: all changes are on the disk.", bir yedeklemenin sonucu ve benzerleri) birkaç saniyeliğine kaydetme metniyle aynı yerde, yani ilk düzenleyicinin alt çubuğunda (iki düzenleyici varsa soldaki ya da üstteki) görünür ve onlardan sonra kaydetme metni geri gelir. Yere sığmayacak kadar uzun bir ileti kısaltılır; tamamını okumak için imleci üzerinde bekletin. Çok dar bir düzenleyicide metnin yerini küçük renkli bir işaret alır (yeşil kaydedildi, sarı henüz kaydedilmedi, kırmızı başarısız) ve metin araç ipucu olarak gösterilir. Yazım modunda ve hiçbir proje açık değilken hiçbir şey gösterilmez.

**{{APP}}'in üzerine yazmayacağı dosyalar** (0.70). Bozuk bir metin dosyası (başı bozuk ya da kesilmiş bir RTF dosyası) veya başka bir program tarafından başka bir karakter kodlamasıyla kaydedilmiş bir dosya (örneğin programın dışında düzenlendikten sonra Windows-1252 ya da UTF-16 olarak kaydedilmiş bir Markdown dosyası) okunabildiği kadarıyla gösterilir ve düzenlenemez; bir ileti bunu söyler ve dosya hiçbir zaman değiştirilmez. Devam etmek için metnini yeni bir belgeye kopyalayın ya da dosyayı onu yazan programda UTF-8 olarak kaydedin, sonra projeyi kapatıp yeniden açın; ya da onu anlık görüntülerinden birine geri döndürün (0.81.1'den beri): bozuk dosya önce projenin *settings/conflicts* klasöründe bir kopya olarak saklanır, anlık görüntünün metni belgenin metni olur; bu metin düzenlenebilir ve yeniden kaydedilir. Belgeleri Birleştir ve Böl böyle bir belgeyi nedenini belirterek reddeder; Projede Değiştir, Tırnakları Dönüştür ve stil değişiklikleri ise onu olduğu gibi bırakır ve adını söyler. Bir komut yine de onu değiştirirse değişen metin kaybolmaz: *settings/conflicts* içinde bir RTF dosyası olarak saklanır ve bir ileti nerede olduğunu söyler (0.81.1). Okunamayan bir zihin haritası dosyasına da aynı şekilde davranılır. Saklanamayan karakterler (yapıştırılan verilerden gelen denetim karakterleri) eskisi gibi dışarıda bırakılır; alt çubukta kısa bir ileti artık bunu belge başına bir kez söyler.

**Bir kaydetme bütün olarak yerine oturur** (0.65). Bir kaydetme birkaç dosyayı birden yazdığında (yeni bir belgenin metni ve proje dosyası, bir metin ve resimleri, Objects sayfası), hepsi önce yerlerinin yanına yazılır, sonra küçük bir günlükte (*settings/journal.json*) listelenir ve ancak ondan sonra birlikte yerlerine konur. Program ya da bilgisayar bu işin ortasında durursa, bir sonraki açılış o kaydetmeyi günlükten tamamlar ya da günlük henüz tamamlanmamışsa projeyi bir önceki kaydetmedeki haliyle bırakır; bir bildirim hangisinin olduğunu söyler. Okunamayan bir günlük (bir disk arızası) hiçbir zaman boş bir günlük sanılmaz: o ve kaydetmesinin dosyaları *settings/conflicts* içindeki bir klasörde saklanır, hiçbir şey yerine konmaz ya da silinmez ve bildirim bunu söyler (0.81.1). Bir eşitleme klasöründe, diğer bilgisayarın bir kaydetmesine ait günlüğe 15 dakika boyunca dokunulmaz (o kaydetme orada hâlâ çalışıyor olabilir ya da dosyaları hâlâ geliyor olabilir): bu bilgisayar bu arada projeyi kaydetmez ve kendiliğinden yeniden dener; diğer bilgisayar o kaydetme sırasında durduysa, bu bilgisayar o 15 dakikadan sonra onu tamamlar (0.81.1). Bir kaydetmenin dosyalarından biri yazılamazsa hiçbiri hiçbir şeyin yerini almaz ve değişiklikleriniz bir sonraki deneme için pencerede kalır. 0.93.0'dan beri, dosyalar yerine konduktan sonra bir klasör eşitlenirken hata bildiren bir sürücü (gerçek bir hata; klasör eşitlemeyi desteklemeyen bir dosya sistemi değil) kaydetmenin başarısız sayılmasına yol açar: günlük kalır, ileti sürücünün bir hata bildirdiğini söyler ve bir sonraki kaydetme ya da açılış kaydetmeyi günlükten tamamlar. Okunamayan bir günlük kenara alınamadığı sürece (*settings/conflicts* klasörüne yazılamıyorsa) hiçbir kaydetme onun üzerine yeni bir günlük yazmaz: kaydetme bekler, iletisi dosyanın adını söyler ve dosya kenara konabilir olur olmaz program kendiliğinden kaydeder. Yolda duran önceki bir kaydetmenin geçici dosyası kenara alınır, asla üzerine yazılmaz. Hiçbir şey yerine konmadan önce başarısız olan bir klasör eşitlemesi temiz bir başarısızlıktır: hiçbir şey değişmemiştir. Hata dosyalar yerine konduktan sonra gelirse program diskte ne olduğunu not eder (böylece yeniden denemeler yanlış "başka yerde değiştirildi" çakışmaları bildirmez ve resimleri iki kez yazmaz) ve her kaydetmede klasörü yeniden eşitler: "Saved" her zaman denetlenmiş bir klasör eşitlemesinden sonra gelir. Proje açılırken bir klasör eşitlemesine daha en baştan hatayla yanıt veren bir dosya sistemi, klasörleri eşitleyemeyen bir sistem olarak hatırlanır ve kaydetmeleri başarısız diye bildirilmez.

{{APP}} açıkken oturumu kapatır ya da bilgisayarı kapatırsanız, sonlanmadan önce her şeyi kaydeder (o sırada yedek alınmaz). Bu kaydetme başarısız olursa (dolu bir disk, kaybolmuş bir sürücü), kaydedilmemiş metinler ve notlar (ve 0.93.0'dan beri bellekteki haliyle proje dosyası) RTF dosyaları olarak bir acil durum klasörüne, *~/.local/share/{{APP}}/Emergency* içine yazılır; bir sonraki başlatma nerede olduklarını ve kaç dosyanın yazıldığını söyler (bir dosya yazılamadıysa bildirim "incomplete" der ve eksik olanı adlandırır). Program beklenmedik biçimde sonlanırsa bir sonraki başlatma size bunu söyler (bölüm 20, *Çökme raporları*).

**Kaydedilemeyen bir projeyi kapatmak** ne yapılacağını sorar; **Açık Tut** varsayılandır. **Metinleri Başka Yere Kaydet…**, henüz kaydedilmemiş metinleri ve notları sizin seçtiğiniz bir klasöre (başka bir sürücü, bir USB bellek) RTF dosyaları olarak yazar ve proje açık kalır. 0.93.0'dan beri bu acil durum kopyası, pencerede olduğu haliyle proje dosyasını (binder, başlıklar ve etiketler), zihin haritası panolarını ve değişen nesneleri de içerir ve yalnızca binder ya da Objects değişmiş olsa bile yapılır. İleti kaç dosyanın yazıldığını "n / m" olarak söyler ve eksik dosyaları ve nedenini adlandırır; dosyası eksik olan bir kopyanın başlığında **incomplete** (bir uyarı işaretiyle) yazar ve asla tamamlanmış bir kopya gibi görünmez. Var olan bir klasör bir daha kullanılmaz: aynı saniyede yapılan ikinci bir kopya numara alır. Kopyadaki proje dosyası, proje dosyasının adına ".copy" eklenerek adlandırılır (böylece klasör yanlışlıkla bir proje sanılmaz); kopyadaki README nasıl geri konacağını söyler: projeyi kapatın, proje klasöründeki proje dosyasını yeniden adlandırın (olduğu gibi kalır), acil durum kopyasındaki dosyayı oraya kopyalayın, proje dosyasının adına yeniden adlandırın ve projeyi yeniden açın. Scrivener biçimindeki bir proje için README, dosyanın çalışma kopyasına ait olduğunu söyler ve geri dönüş yolu vermez. Sonuçta hiçbir şey kaydedilmemiş durumda değilse (bu arada bir yeniden deneme başarılı oldu), ileti her şeyin kaydedildiğini ve kopya gerekmediğini söyler. **Kaydetmeden Kapat** projeyi kapatır ve bu değişiklikleri kaybeder.

### Çalışmanız nerede durur

Bir proje, *YourTitle*{{SUFFIX}} adlı bir klasördür. İçinde her belge sıradan bir metin dosyasıdır; böylece yazdıklarınız {{APP}} olmadan da okunabilir kalır. Projeyi açmak için klasörün içindeki *YourTitle*{{FILE}} dosyasına çift tıklayın. Proje klasörünün içindeki dosyaları elle yeniden adlandırmayın ya da taşımayın.

Bir belgenin dosyasını {{APP}}'in dışında bulmak için proje klasöründeki *contents-by-title.txt* dosyasını açın (0.65): binder'daki her belgeyi başlığıyla, yerine göre girintili olarak ve metnini, notlarını ya da medyasını tutan dosyalarla (örneğin *content/<id>/text.md*) listeler. {{APP}} bunu binder'ı değiştiren her kaydetmede yazar ve kendisi asla okumaz. (Scrivener biçimindeki bir projede böyle bir liste yoktur; belgelerini Scrivener'ın *.scrivx* dosyası adlandırır.)

Bir proje aynı anda yalnızca tek pencerede açık olmalıdır. Zaten başka bir pencerede ya da başka bir bilgisayarda (örneğin eşitlenen bir klasör aracılığıyla) açıksa, {{APP}} açmadan önce sizi uyarır. {{APP}}'i, programın bir penceresinde zaten açık olan bir projeyle yeniden başlatmak (dosya yöneticisinden ya da bir masaüstü kısayolundan) projeyi iki kez açmak yerine o pencereyi öne getirir; başka bir proje kendi penceresinde açılır.

**Eşitlenen bir klasördeki proje** (OneDrive, Dropbox, Nextcloud, Syncthing ya da başka herhangi bir eşitleme programı), her seferinde yalnızca bir tanesinde olmak üzere birkaç bilgisayarda kullanılabilir. Onu bir bilgisayarda kapatın ve diğerinde açmadan önce eşitleme programının bitirmesini bekleyin. {{APP}}, eşitleme programı hangisi olursa olsun, dosyalar düzeyinde bu konuda yardımcı olur:

- Bir proje açıkken, içindeki küçük bir kilit dosyası bilgisayarın adını taşır ve birkaç dakikada bir yenilenir. Bilgisayarın adını, o bilgisayardaki kullanıcı adınızı, programın sürümünü ve işlem numarasını, projenin ne zaman açıldığını ve kilidin en son ne zaman yenilendiğini, bir de bilgisayarları birbirinden ayıran bir kodu (bir adın okunabilirliği olmayan bir özet değeri) içerir. Paylaşılan klasöre erişimi olan herkes bunları görebilir. **Dosya > Seçenekler > Genel > Kaydetme** içinde **Paylaşılan klasörlerde bu bilgisayarın adı** kendi seçtiğiniz bir adı ("Laptop") ayarlar; kilit dosyalarında, saklanan sürümlerin adlarında ve *settings/ui-<computer>.json* dosyasında bilgisayarın adı yerine bu kullanılır ve kullanıcı adınız o zaman dışarıda bırakılır. Başka bir bilgisayarda projeyi açmak o zaman o bilgisayarın adını, projeyi ne zamandan beri açık tuttuğunu ve son yaşam belirtisini gösterir; **Yine de Aç** ya da **İptal**. 15 dakikadan uzun süredir yenilenmemiş bir kilit (proje orada kapatılmadı ya da o bilgisayar kapalı) artık geçerli olmayan bir kilit olarak gösterilir ve **Aç** varsayılandır.
- Proje, burada açıkken başka bir bilgisayarda açılırsa, bir bildirim bunu söyler ve projeyi burada kapatmayı önerir.
- Projenin bir dosyası, {{APP}} onu okuduktan sonra başka bir yerde (başka bir bilgisayarda eşitleme programı aracılığıyla ya da başka bir program tarafından) değiştirilmişse ve burada da değiştirilmişse, sizinki kaydedilmeden önce diğer sürüm saklanır: belgenin bir **anlık görüntüsü** olarak (denetçinin Anlık Görüntüler sekmesinde karşılaştırın) ve projenin *settings/conflicts* klasöründe bir kopya olarak. Bir bildirim hangi belgeler olduğunu söyler. Diğer sürüm saklanamıyorsa (örneğin disk dolu olduğu ya da *settings/conflicts* klasörüne yazılamadığı için), sizinki onun üzerine kaydedilmez: kaydetme dosyayı adlandıran bir iletiyle başarısız olur ve değişiklikleriniz, diğer sürüm saklanabilene kadar kaydedilmemiş kalır. Burada yalnızca baktığınız bir belge, başka bir yerden gelen daha yeni bir sürümün üzerine asla kaydedilmez.
- Hiçbir şeyi değiştirmeden bir projeyi açıp kapatmak dosyalarının hiçbirini değiştirmez; pencere düzeni ve her belgede bulunduğunuz yerler bilgisayar başına saklanır (*settings/ui-<computer>.json*). Scrivener biçiminde *.scriv* de ellenmez ve açıkken {{APP}}'in bir kilit dosyasını (*quillbinder.lock*) alır; Scrivener bunu okumaz. *.scriv* içine her kaydetmeden önce {{APP}}, Scrivener'ın kendi kilidine bakar: Scrivener projeyi {{APP}}'ten sonra açtıysa *.scriv* içine artık hiçbir şey yazılmaz (bir bildirim bunu bir kez söyler, sonra alt çubukta kısa bir ileti); değişiklikleriniz {{APP}}'in çalışma kopyasında kalır. Projeyi Scrivener'da kapatın ve yeniden kaydedin (**Dosya > Kaydet**). Projeyi önce burada kapatırsanız, değişiklikler onu burada bir sonraki açışınızda *.scriv* içine gider; bu arada Scrivener bir şeyi değiştirdiyse onun sürümü okunur.
- Proje, dosyası bu bilgisayara henüz ulaşmamış (eşitleme programı henüz bitmedi) bir belge için metin listeliyorsa, açıldıktan sonra bir bildirim onu adlandırır; o belge boş gösterilir, düzenlenemez ve onun yerine hiçbir şey kaydedilmez. Projeyi kapatın, eşitleme programının işi bitirmesini bekleyin ve yeniden açın. Aynısı yalnızca kısmen okunabilen bir metin için de geçerlidir (0.65).
- Başka bir yerde değiştirilen bir dosya, boyutunu ve zamanını korumuş olsa bile fark edilir (bazı eşitleme programları zamanı kendileri ayarlar): {{APP}}, içeriğini okuduğu ya da yazdığı şeyle karşılaştırır (0.65).
- İçerik öbür yönde de belirleyicidir (0.92.0): **yalnızca zamanı** değişen bir dosya başka bir sürüm değildir. Bazı eşitleme programları, yüklemeleri bittiğinde, {{APP}} bir dosyayı yazdıktan birkaç saniye sonra dosyanın zamanını yeniden ayarlar; bunun için hiçbir şey saklanmaz, bildirim gelmez ve dosya yeniden yazılmaz. Bir kaydetmenin yazacağı şeyi zaten içeren bir proje dosyasına da dokunulmaz; böylece hiçbir şeyi değiştirmeyen bir kaydetme eşitleme programına taşıyacak bir şey vermez.
- **Diğer Sürümler Saklandı** bildiriminde **{{APP}} Kapatılana Kadar Bir Daha Gösterme** düğmesi vardır. Diğer sürümler yine anlık görüntüler olarak ve *settings/conflicts* içinde kopyalar olarak saklanır; yalnızca bildirim, programdan çıkana kadar görünmez. Bir sonraki başlatma onu yeniden gösterir.
- **Başka bir aygıtın üzerinde çalıştığı şeyde bir asma kilit (0.92.0).** Bir proje açıkken her aygıt, kilit dosyasının yanında hangi belgeleri bir düzenleyicide açtığını ya da projeyi açtığından beri hangilerini değiştirdiğini de not eder. İkinci bir aygıt projeyi açtığında, binder'ı bu belgelerin satırlarının sağ ucunda ve çevrelerindeki klasörlerde küçük bir asma kilit gösterir. Asma kilidin üzerine gelin: araç ipucu aygıtı ve projeyi ne zaman açtığını adlandırır. Bu bir ipucudur, bir koruma değildir: hiçbir şey engellenmez ve belgeyi yine de düzenleyebilirsiniz. İşaret eşitleme programınız aracılığıyla taşınır, bu nedenle birkaç dakika eski olabilir; her iki aygıtta da 0.92.0 ya da daha yenisini gerektirir (Scrivener hiçbir şey bildirmez). Aygıtı 15 dakikadan uzun süredir hiçbir yaşam belirtisi göstermemiş bir liste gösterilmez. Not, aygıt başına küçük bir dosyadır (*settings/lock-work-…json*; Scrivener biçimindeki bir proje için *.scriv* içinde *quillbinder.lock-work-…json*); aygıtın adını kilit dosyasındaki gibi, zamanı ve belgelerin iç numaralarını içerir; başlık ve metin içermez. Yalnızca liste değiştiğinde yazılır, asla tuş vuruşu başına yazılmaz, proje kapatıldığında kaldırılır ve yedeklerin dışında bırakılır.
- **Scrivener biçimi, iki bilgisayar** (0.65): proje burada açıkken *.scriv* içine başka bir yerde (başka bir bilgisayardaki Scrivener) eklenen belgeler saklanır ve onları adlandıran binder'ın yerine yenisi konmaz; bir bildirim projeyi kapatıp yeniden açmanızı ister, bu da hem onların hem sizin belgelerinizi gösterir. *.scriv* içinde başka bir yerde değiştirilen resimler, PDF'ler, ses ve video dosyaları ve zihin haritaları olduğu gibi saklanır (eskiden buradaki sürüm onların üzerine kopyalanırdı); biri her iki tarafta da değiştirilmişse, diğer sürüm önce *settings/conflicts* içinde saklanır.

**Scrivener biçimindeki** bir proje bunun yerine *YourTitle.scriv* adlı bir klasördür; onu *.scrivx* dosyasıyla açın. {{APP}} değişikliklerinizi ona kaydeder, böylece proje bir Scrivener projesi olarak kalır. **Dosya > Farklı Kaydet…** bir projeyi başka bir adla ya da başka bir yerde, her iki biçimde de kaydeder (bölüm 14).

### Gizlilik: bilgisayarınızdan ne çıkar

{{APP}} internete yalnızca bir şey için bağlanır: güncelleme aramak. Küçük bir dosya çeker, sizinle ilgili hiçbir şey göndermez ve Yardım menüsünden kapatabilirsiniz. (Bölüm 17, *Güncelleme denetimi*, tam olarak ne ve ne zaman çekildiğini söyler. 0.92.0'dan önce program hiç güncelleme aramazdı.) İnternete çıkan üç şey daha vardır ve yalnızca siz başlattığınızda çıkar:

- **Dosya > İçe Aktar > Web Sayfası…** adını verdiğiniz sayfayı ve resimlerini o siteden çeker. Bir tarayıcı gibi, yaygın bir tarayıcının adını (Linux'ta Firefox), sisteminizin dillerini (sayfanın dili için) ve her bağlantıda olduğu gibi bilgisayarınızın internet adresini gönderir.
- Çeviri Yardımcısı eklentisi için **bir dil modeli indirmek** (bölüm 10), onu {{APP}}'in GitHub'daki deposunun sürümlerinden çeker. Eklentinin adını ("Quillbinder-Translation-Addon") ve her bağlantıda olduğu gibi internet adresinizi gönderir.
- **Bir konuşma modeli indirmek** (Speech Transcription eklentisi, bölüm 10), modeli Hugging Face'ten (huggingface.co, ABD'de bir şirket) ya da seçeneklerinde ayarladığınız adresten çeker. Programın adı olarak "{{APP}}/<version>" ve her bağlantıda olduğu gibi internet adresinizi gönderir. Bir model yalnızca sağlama toplamı bilinen sağlama toplamıysa kurulur; başka bir adresten gelen ya da kendi indirdiğiniz bir dosya yalnızca siz onayladıktan sonra kullanılır.

Başkalarının çalışmanızdan görebildikleri:

- **Paylaşılan klasörlerdeki projeler** açıkken bir kilit dosyası taşır (yukarıya bakın): bilgisayar adı, kullanıcı adı ve zamanlar. Bunları dışarıda bırakmak için **Dosya > Seçenekler > Genel > Kaydetme** içinde kendi adınızı ayarlayın.
- **Yazar Bilgileri** (**Dosya > Seçenekler** içinde ad, adres, telefon, e-posta) bu bilgisayardaki programın ayarlarında kalır; hiçbir zaman bir projenin kendi ayarlarına girmez. Bir müsveddeye yalnızca derleme biçimi yer tutucuları (<$author>, <$address> ...) kullandığında ulaşır.
- **Silinen metin her yerden gitmiş değildir**: anlık görüntüler bir belgenin önceki sürümlerini saklar (**Proje > Eski Anlık Görüntüleri Sil** onları kaldırır) ve yedekler bütün projeyi olduğu gibi saklar (bölüm 15). Yedekler parolasız sıradan ZIP dosyalarıdır; eşitlenen bir klasörün içindeki bir yedek klasörü, eşitleme programı tarafından yüklenir.
- **Dosyalarınız {{APP}} tarafından şifrelenmez** (0.65): metinler, notlar, projenin ayarları ve yedekler, bulundukları klasörü okuyabilen herkesin okuyabileceği sıradan dosyalardır. Bir müsveddeyi gizli tutmak için projeyi ve yedeklerini şifreli bir diskte tutun (Kubuntu'nun yükleyicisi tüm diski şifrelemeyi önerir; bir USB bellek ya da harici bir sürücü için KDE'nin Plasma Vaults özelliği ya da LUKS ile şifrelenmiş bir sürücü) ve yedek klasörünü denetlemediğiniz paylaşılan klasörlerin dışında tutun.
- **Çökme raporları**, siz birini kendiniz kopyalayana ya da gönderene kadar bu bilgisayarda kalır (bölüm 20). Programın durumunu anlatırlar ve açık projeyi ve klasörünü adlandırırlar. Windows'ta her raporun yanında bir bellek dökümü saklanır; yazmakta olduğunuz metni içerebilir ve rapor bunu söyler.

### {{APP}}'i uygulama menüsüne eklemek

Program dosyasını ({{APP}}.AppImage) ilk kez başlattığınızda, ilk başlatma sayfası {{APP}}'i uygulama menüsüne eklemeyi önerir. Bundan sonra onu başka herhangi bir program gibi başlatabilirsiniz ve bir projeye çift tıklamak onu açar. Hiçbir kutu girişi onaylamaz; bir başarısızlık bildirilir. Aynı sayfa (ve **Yardım > Uygulama Menüsüne Ekle**) **AppImage'ı ~/Applications klasörüne taşımayı** da önerir (dosya henüz orada değilse başlangıçta işaretlidir); böylece menü girişi onu dosyanın kalacağı sabit bir yerden başlatır (onay kutusu bunu bu sözlerle söyler, parantezsiz); taşıma başarısız olursa (dolu bir disk, o adda bir dosyanın zaten orada olması) hiçbir şey kaybolmaz: dosya olduğu yerde kalır, giriş oraya işaret eder ve bir ileti nedenini söyler. AppImage'ı sonradan kendiniz taşırsanız giriş hâlâ eski yeri adlandırır: yeniden **Yardım > Uygulama Menüsüne Ekle** komutunu kullanın; o, dosyanın geçerli yerini yazar (eski bir giriş değiştirilir, ikiye katlanmaz). Bir taşımadan sonra, program güncellemeleri, *.before-update* kopyası ve **Yardım > Sorun Giderme ve Geliştirme > Sistem Bilgisi** hemen yeni yeri kullanır.

Yardım menüsünün tek komutu olan **Yardım > Uygulama Menüsüne Ekle**, girişin durumunu izler: giriş yokken onu ekler ve bir giriş olduğunda *Uygulama Menüsünden Kaldır* adını alır ve eklemenin yaptığı şeyi (giriş dosyasını, simgeyi ve dosya türlerini) kaldırır, AppImage'ı olduğu yerde bırakır. Program dosyasını kendiniz taşırsanız, girişi kaldırın ve yeniden ekleyin.

### Windows'ta: her şey tek bir klasörde

{{APP}}'in Windows sürümü (deneysel, 0.91.1'den beri yayımlanıyor), istediğiniz yere açtığınız bir klasördür, örneğin *C:\Users\<your name>\Quillbinder*; kurulum ya da yönetici hakları gerektirmez. Ayarlarını, eklentilerini, program güncellemelerini, kurulu yazı tiplerini ve çökme raporlarını o klasörün içindeki *data* klasöründe tutar ve Windows'un kendi klasörlerinde ya da kayıt defterinde hiçbir şey bırakmaz. Klasörün tamamı taşınabilir ya da kopyalanabilir (bir USB belleğe ya da başka bir bilgisayara); silmek programı kaldırır. Programın yazamadığı bir klasör (*Program Files* gibi) reddedilir ve bir ileti onu bunun yerine nereye koyacağınızı söyler. Program güncellemeleri **Yardım > Eklentiler ve Güncellemeler > Güncelleme veya Eklenti Kur** ile kurulur; yeniden indirdiğiniz bir sürüme geçmek için onu açın ve *data* klasörünü yeni klasöre kopyalayın. Projeleriniz tuttuğunuz yerde kalır.

Bu klasörü edinmenin iki yolu vardır (0.90.4). **Kurulum programı** (*...-setup.exe*) bir klasör sorar ve {{APP}}'i oraya açar; yönetici hakları gerektirmez ve bilgisayarda başka hiçbir şeyi değiştirmez: Uygulamalar altında giriş yok, kayıt defterinde hiçbir şey yok, Başlat menüsü girişi yok (aşağıda anlatılan rehber bunları önerir). Daha yeni bir sürümü aynı klasöre açmak *data* klasörünü, yani ayarlarınızı ve eklentilerinizi korur. **Zip dosyası** aynı klasörü içerir: onu eksiksiz açın. Windows, imzalı olmadığı için her iki dosya için de uyarabilir (SmartScreen: *More info*, ardından *Run anyway*).

Windows'un kendisi yüzünden iki şey Linux'tan farklıdır (0.90.4). **AltGr** tuşu, beklediğiniz gibi kendi karakterini yazar: Almanca bir klavyede AltGr+7, 8, 9, 0 ve < tuşları { [ ] } ve | karakterlerini verir, ancak {{APP}}'in bu tuşlarla Ctrl+Alt üzerinde komutları vardır; komutlar için sol Ctrl ve Alt tuşlarına basın. Ve yalnızca bir an için gereken dosyalar (görüntülemek için açılan kaydedilmiş bir web sayfası ya da tarayıcı için kopyası) Windows'un geçici klasöründe tutulur ve {{APP}} kapanınca kaldırılır. Speech Transcription, Windows'ta grafik kartını değil işlemciyi kullanır.

İlk başlatmada bir rehber, programın dosyalarını nerede tuttuğunu söyler ve klasörü bir eşitleme hizmetinin klasörünün (OneDrive, Dropbox, Google Drive ...) içindeyse ya da yolu uzunsa uyarır. Ayrıca şunları sorar; bunları **Yardım > Sorun Giderme ve Geliştirme > Bu Bilgisayar** ve **Dosya > Seçenekler > Genel > Bu Bilgisayar** yeniden gösterir:

- **Yedekler**: standart bir yedek klasörü yoktur. Siz bir tane seçene kadar otomatik yedek alınmaz; ilk düzenleyicinin alt çubuğu kaydetme metninin yanında **Yedek yok** gösterir (klasörü seçmek için ona tıklayın) ve çalışacak ilk yedek bir klasör sorar. En iyisi başka bir sürücüdür.
- **Karalama Defteri notları ve yazıya dökümler**: Documents içinde (standart), programın klasöründe ya da kendi klasörünüzde. Bir değişiklik her iki araca da yeni yeri verir; daha önce yazdıkları yerde kalır.
- **Windows**: Başlat menüsünde bir giriş, masaüstünde bir kısayol ve proje dosya türleri (*.qbx* dosyaları {{APP}} ile açılır ve Scrivener'ın *.scrivx* dosyaları için **Open with** altında önerilir). Bunlar, programın kendi klasörünün dışına koyduğu tek şeylerdir; her biri yalnızca siz eklediğinizde konur ve her biri **Kaldır** ile kaldırılır. Klasörü silmeden ya da taşımadan önce bunları kaldırın.

**Biçim > Yazı Tipi > Yazı Tipi Dosyalarını Kur** ile kurulan yazı tipleri programın klasöründe kalır ve yalnızca {{APP}}'e aittir; başka programlar onları görmez.

Program kodu olan eklentilerin kendi Windows sürümleri vardır (dosya adları *-windows* ile biter): **Media Playback** Windows'un kendisinin oynattığını oynatır, **Extra Video and Audio Formats** FFmpeg ekler ve **Speech Transcription** konuşmayı işlemcide yazıya döker (grafik kartları Windows'ta henüz kullanılmaz). **Çeviri Yardımcısı** Windows'ta da tam sürümün parçasıdır (0.92.0). Program kodu olmayan eklentiler (temalar, şablonlar, derleme biçimleri) her sistemde aynı dosyalardır.

### Android tabletlerde

{{APP}}'in Android sürümü (deneysel, 0.91.3'ten beri yayımlanıyor), Android 9 veya üstü (64 bit ARM) olan tabletler için bir uygulamadır. Bilgisayardakiyle aynı pencerelere sahiptir, ancak parmaklar için yapılmıştır: bir parmak kaydırır ve dokunur, iki parmak yakınlaştırır, bir şeyin üzerinde yarım saniye bekleyen bir parmak onun sağ tık menüsünü açar (bir metinde, seçili metne dokunmak bunu yapar) ve araç çubukları ekranın gerektirdiği kadar satıra katlanır. APK dosyasını tablete indirin ve açın; Android, tarayıcının ya da Dosyalar uygulamasının uygulama kurup kuramayacağını sorar; bu kurulum için buna izin verin. Yeni bir sürüm yeni bir APK olarak gelir ve eskisinin üzerine kurulur, ayarlarınızı ve eklentilerinizi korur; **Yardım > Eklentiler ve Güncellemeler > Güncellemeleri Şimdi Denetle** yeni bir sürüm olduğunda size söyler ve indirmesini tarayıcıda açar (daha yeni eklentiler {{APP}}'in içinde kurulur).

Harici bir klavye ve fare ile (0.92.0'dan beri): Ctrl + fare tekerleği yakınlaştırır ve Ctrl + tıklama ya da Shift + tıklama, bir bilgisayardaki gibi birkaç belgeyi seçer. İki bölme arasındaki çizgi bir tutamaç gösterir; birkaç piksel yanına basılan bir fare onu da götürür. Android'in **Geri** düğmesi öndeki menüyü, ipucunu ya da pencereyi kapatır; her pencerenin onu kapatan bir düğmesi vardır ve ekrandan büyük bir pencere kayar. Bir tablette sabitlenmiş zihin haritasının raptiyesi seçili binder satırında görünür (ona tıklayın ya da satırın sağ tık menüsünü kullanın: *Bu Öğenin Zihin Haritasını Sabitle*, *Sabitlenen Zihin Haritasını Bırak*) ve bir satırın asma kilidine dokunmak kimin onun üzerinde çalıştığını gösterir. Zihin haritasının genel görünümü üzerindeki bir parmak fare gibi çalışır: bir dokunuş görünümü oraya taşır, bir parmak gösterilen bölümün dikdörtgenini sürükler ve köşesindeki tutamaç boyutunu değiştirir. Bir tablette **Dosya > Farklı Kaydet…** önce biçimi sorar, sonra dosya penceresini açar.

İlk başlatmada {{APP}}, tabletin dosyalarına erişim ister. Bununla projeler *Documents/Quillbinder* klasörüne, yedekler *Documents/Quillbinder/Backups* klasörüne, dışa aktarmalar *Documents/Quillbinder/Exports* klasörüne ve Karalama Defteri notları *Documents/Quillbinder/Scratch Pad* klasörüne gider; diğer uygulamalar, bir eşitleme uygulaması ve kabloyla bağlı bir bilgisayar buralara ulaşabilir; düğme Android'in *All files access* sayfasını açar, orada {{APP}} için açarsınız. Bu olmadan her şey, hiçbir başka uygulamanın görmediği ve Android'in uygulamayla birlikte sildiği uygulamanın kendi klasöründe kalır; *File > Files on This Tablet* yeniden sorar ve her şeyin nerede olduğunu söyler. Android uygulamayı kenara aldığında (Ana ekran düğmesi, başka bir uygulama), {{APP}} hemen kaydeder.

Yazım denetimi sözlükleri (İngilizce ve Almanca; diğer diller eklentidir, bkz. bölüm 4), yazım araçları, web sayfaları ve eski Word dosyaları için içe aktarıcılar ve (0.92.0'dan beri) Çeviri Yardımcısı uygulamanın parçasıdır; Çeviri Yardımcısı bir sürümün ilk başlatmasında kurulur ve bir dil modelini yalnızca siz seçtiğinizde indirir. Android için eklentiler (dosya adları *-android-arm64* ile biter) **Yardım > Eklentiler ve Güncellemeler > Güncelleme veya Eklenti Kur** ile kurulur; dosya penceresi İndirilenler klasöründe açılır ve eklenti gerektirdiğinde {{APP}} yeniden başlar. Linux ya da Windows için yapılmış paketler ve program güncellemeleri nedeni belirtilerek reddedilir: uygulamanın kendisi yalnızca yeni bir APK ile güncellenir. Android sürümünde henüz olmayanlar: Speech Transcription, düzenleyicide ses ve video ve çökme raporları. {{APP}}'in içinde kopyalanan metin, yapıştırdığınızda biçimlendirmesini korur; Android'in panosu yalnızca düz metin taşır, bu nedenle başka uygulamalara kopyalanan metin düz olarak ulaşır.

---

## 2. Ana pencere

Pencerenin üç sütunu vardır:

| Alan | Ne yapar |
|------|--------------|
| **Binder** (sol) | Bütün projenizin içindekiler tablosu: her belge ve klasör. |
| **Düzenleyici** (orta) | Binder'da seçtiğinizi gösterir: metin, dizin kartlarından oluşan bir mantar pano ya da bir anahat. |
| **Denetçi** (sağ) | Geçerli belge hakkında bilgi: özet, notlar, yer imleri, üst veri, anlık görüntüler ve yorumlar. |

{{APP}} 1600 px'ten dar bir ekranda ilk kez başladığında pencere büyütülmüş olarak açılır. Pencerenin boyutu, yeri ve durumu (büyütülmüş ya da değil) her değişiklikten bir an sonra kaydedilir, böylece bir çökme bunları kaybettirmez. Daha fazla yer kazanmak için binder'ı **Ctrl+Alt+B** ile, denetçiyi **Ctrl+Alt+I** ile gizleyin. Bu, pencere genişliği yüksekliğinden büyük olduğu sürece geçerli olan pencere düzenidir; genişliğinden yüksek bir pencere aynı üç alanı farklı düzenler (sonraki bölüm). Üçünün ve ayrıca mantar panonun, anahatın ve yazım modunun renkleri ve yazı tipleri temadan (**Görünüm > Temalar**) gelir ve alan alan değiştirilebilir (bölüm 16), alana sağ tıklayarak da (**Dış Görünüm**). Her proje kendi görünümünü korur.

Üstteki **araç çubuğu** soldan sağa şunları içerir: **Görünüm** düğmesi (binder, koleksiyonlar, denetçi, yerleşimler), yeni belge (klasörler, şablonlar ve içe aktarma için bir menüyle), yeni klasör, çöp kutusu, **geri al** ve **yinele**, **arama** düğmesi (proje araması; yanındaki ok diğer arama araçlarını listeler: Belgede Ara, Projede Değiştir, Biçimlendirmeye Göre Bul), **hızlı arama** alanı, üç görünüm düğmesi (metin, mantar pano, anahat), proje yer imleri, hızlı başvuru, yorumlar ve dipnotlar, derleme, yazım modu, Speech Transcription'ın **mikrofonu** (bu eklenti kurulu olduğunda) ve denetçi düğmesi. İçinde ne olacağına siz karar verirsiniz (aşağıdaki "Kendi araç çubuklarınız" bölümüne bakın).

Altındaki **biçim çubuğu** stili, yazı tipini, boyutu, satır aralığını, varsayılan metin biçimlendirmesini, yazı tipleri penceresini, kalın, italik, altı çizili, üstü çizili, vurgulama, metin rengi, hizalama, listeler, paragraf öncesi ve sonrası boşluğu, ilk satırın ve bütün paragrafın girintisini, yorumları, dipnotları, satır içi açıklamaları ve resimleri içerir. İki çubuğu **Görünüm > Araç Çubukları** ile gösterin ya da gizleyin.

**İletişim pencereleri.** Programın açtığı bir pencere (Seçenekler, Derle, Proje Hedefleri ...) masaüstünün panelleri düşüldükten sonra ekrandan asla büyük olmaz: küçük bir dizüstü ekranında içeriği kayar ya da küçülür ve düğmeleri görünür kalır. Kapandığında klavye eski yerine, genellikle metne döner; böylece hemen yazmaya devam edebilirsiniz.

**Neredeyim?** Metnin yanındaki boşluğa çift tıklayın (sütunun yanındaki kâğıda ya da çevresindeki alana): binder, tıklamanın yanındaki belgeyi seçer, üstündeki klasörleri açar ve satırını iki kez turuncu yanıp söndürür. Düzenleyici gösterdiğini korur. **Gezin > Binder'da Göster** (**Ctrl+Shift+R**) imlecin bulunduğu belge için aynısını yapar.

Her düzenleyicinin **alt bilgisi** sözcük ve karakter sayısını (yalnızca seçimi saymak için metin seçin) ve belgenin hedefini gösterir. Ayrıntılar için sayıya tıklayın. Düzenleyici bir klasör (ya da alt belgeleri olan bir belge) gösterdiğinde alt bilgi klasörün **toplamını** da gösterir: kendi metninin ve içindeki her şeyin sözcükleri, örneğin "Folder: 12,345 words". Mantar pano ve anahat aynı toplamı alt bilgilerinin ortasında gösterir; birkaç öğe gösteriliyorsa hepsinin toplamıdır. Bir toplama tıklamak, bir metnin sayısı gibi ayrıntılarını (sözcükler, karakterler, boşluksuz karakterler, paragraflar ve sayfalar) gösterir. Denetçi onu bir klasörün üst verisinde **Toplam** altında gösterir ve binder'da bir klasörün üzerinde fareyi bekletmek de onu gösterir (klasörün kendi metni varsa kendi sözcükleriyle birlikte).

### Menüler

| Menü | Neler içerir |
|------|---------------|
| **Dosya** | Projeler, kaydetme, proje biçimi ({{APP}} ya da Scrivener), yedekler, içe ve dışa aktarma, bir Word ya da LibreOffice dosyasıyla karşılaştırma ve aktarma, derleme, yazdırma, dosya dönüştürme, seçenekler |
| **Düzenle** | Geri al, pano, seçme, bulma ve değiştirme (metinde, projede, biçimlendirmeye göre), binder'da belgeleri taşıma, yazım araçları |
| **Ekle** | Resimler, tablolar, belgelere ve nesnelere bağlantılar, web bağlantıları, yorumlar, dipnotlar, medya zaman damgaları, otomatik numaralar, yer tutucular, özel karakterler, tarih ve saat |
| **Görünüm** | Üç görünüm modu, bölünmüş düzenleyici (**Düzenleyiciyi Böl**), binder'ın, düzenleyicinin ve denetçinin yerleri ve kayıtlı yerleşimler (**Yerleşim**), renk temaları (**Temalar**), binder, koleksiyonlar ve denetçi (ve **Binder'da Hızlı Ekleme Tuşları**), araç çubukları, metin düzenleme seçenekleri (sayfa görünümü, cetvel, daktilo kaydırma), mantar pano ve anahat seçenekleri, etiket renkleri, yakınlaştırma, yazım modu, tam ekran |
| **Gezin** | Geri ve ileri, önceki ve sonraki belge, belgeleri diğer düzenleyicide, referans bölmesinde ya da hızlı başvuru olarak açma, düzenleyici kilidi, binder'ın hangi düzenleyiciye yüklediği, denetçi sekmeleri (hepsi, araç ipuçlarının adlarıyla: **Özet ve notlar**, **Yer İmleri**, **Üst Veri**, **Anlık Görüntüler**, **Yorumlar ve dipnotlar**, zihin haritası, nesneler ve yazım araçlarının sekmeleri) |
| **Proje** | Yeni belgeler, proje ayarları (ayrıca bir dosyaya kaydedilir ve dosyadan yüklenir), hedefler, anahtar sözcükler, istatistikler, yazma geçmişi, çöp kutusu, eski anlık görüntüleri silme |
| **Belgeler** | Anlık görüntüler, yeniden adlandırma, çoğaltma, bölme, birleştirme, dönüştürme, simge, etiket, durum, yer imleri |
| **Biçim** | Yazı tipi (yazı tipi kurulumuyla), paragraf, stil, listeler, vurgulama, biçimlendirmeyi kopyalama, revizyon modu |
| **Pencere** | Karalama defteri, hızlı başvuru pencereleri |
| **Yardım** | **Komut Bul**, bu kılavuz, Hızlı Başlangıç Kılavuzu, karşılama projesi, tüm yer tutucuların listesi, klavye kısayollarının listesi, zihin haritaları yardımı, **Eklentiler ve Güncellemeler** alt menüsü (Eklenti Yöneticisi, Güncelleme veya Eklenti Kur, güncelleme denetimi), {{APP}}'i uygulama menüsüne ekleme (ve kaldırma), **Sorun Giderme ve Geliştirme** alt menüsü (sistem bilgisi, Bu Bilgisayar, çökme raporları, Eklenti Klasörünü Aç, geliştirici kılavuzu), Sorun Bildir, üçüncü taraf lisansları, Hakkında |

**Komut Bul** (**Yardım > Komut Bul…**), bir arama alanı ve menülerdeki tüm komutların, her biri menüsü ve kısayoluyla birlikte listesini içeren bir pencere açar. Bir adın bir bölümünü yazın (birkaç sözcük, herhangi bir sırada, aramayı daraltır); **Return** ya da çift tıklama seçilen komutu çalıştırır. O anda kapalı olan bir komut gri görünür ve çalışmaz. Kendi kısayolu yoktur.

Bir alt menü kısa adlar gösterir: **Dosya > İçe Aktar > Dosyalar…**, **Belgeler > Böl > Seçimden**, **Gezin > Aç > Hızlı Başvuru Olarak**. Bir komutun kısayolu yanında gösterilir ve değiştirilebilir (bölüm 19). Bir menünün her girişinin kendine ait bir harfi vardır (**Alt** tuşunu basılı tuttuğunuzda altı çizilir), böylece **Alt** ve harfler her komuta ulaşır.

Açık bir proje olmadan **Ekle**, **Gezin**, **Proje**, **Belgeler** ve **Biçim** menüleri grileşir, bir metin ya da binder gerektiren diğer menülerin komutları da öyle (Bul, Yakınlaştır, düzenleyici yerleşimi, Binder'a Odaklan ...). Proje olmadan çalışanlar: projeleri oluşturma, açma ve içe aktarma, dosyaları dönüştürme, güncellemeleri ve eklentileri kurma, seçenekler, temalar ve yerleşimler, proje gerektirmeyen yazım araçları ve Yardım.

### Pencerenin biçimine göre bölmeler

Pencere binder'ı, düzenleyiciyi ve denetçiyi biçimine göre düzenler ve biçim değiştiğinde kendiliğinden değişir (pencereyi genişliğinden yüksek olacak şekilde sürüklersiniz ya da bir tableti döndürürsünüz). Kare bir pencere geniş sayılır.

- **Geniş pencere** (yüksekliğinden geniş ya da kare): üç sütun. Sıralarını **Görünüm > Yerleşim > Geniş pencere** altında seçin: binder, düzenleyici ve denetçinin altı sıralamasından herhangi biri. Genişlikleri sütunlar arasındaki çizgilerle ayarlanır.
- **Dik pencere** (genişliğinden yüksek): araç çubuklarının altında iki satır. Bir satır, pencerenin tam genişliğinde düzenleyicidir; diğeri binder ve denetçiyi yan yana tutar. **Görünüm > Yerleşim > Dik pencere** altında, **Düzenleyici Ortada** düzenleyicinin satırını diğerinin üstüne, **Düzenleyici Altta** altına koyar; **Binder Solda** ve **Binder Sağda** binder'ın tarafını seçer (denetçi diğerini alır). İki satırın yükseklikleri ve binder ile denetçinin genişlikleri aralarındaki çizgilerle ayarlanır. Ekran klavyesi göründüğünde (Android), binder ve denetçinin satırı yüksekliğini korur; farklı olmasını istiyorsanız çizgiyi taşıyın.
- Gizli bir binder ya da denetçi, dik satırda yerini diğerine, tam genişlikte bırakır; ikisi de gizliyse düzenleyici pencerenin tamamına sahiptir. İki düzenleyici (**Görünüm > Düzenleyiciyi Böl**) düzenleyicinin yerinin içinde kalır.
- **Görünüm > Yerleşim > Reset Layout**, pencerenin şu an sahip olduğu biçimi başlangıç değerlerine geri koyar: geniş bir pencerede binder, düzenleyici, denetçi; dik bir pencerede ortada düzenleyici ve solda binder.

Her iki grup da her zaman menüdedir; her biri pencere o biçime sahip olduğunda etkili olur ve her biçim bıraktığınız gibi (yerler ve boyutlar) geri gelir. Seçimler bu bilgisayara ya da tablete aittir ve programın ayarlarında tutulur, asla bir projede değil: bilgisayarlar arasında paylaştığınız bir proje her aygıtın düzenini korur ve onun hakkında projenin dosyalarına hiçbir şey yazılmaz.

### Yerleşimler

Bir yerleşim pencerenin nasıl düzenlendiğini hatırlar: binder, koleksiyonlar, denetçi, araç çubuğu ve biçim çubuğunun gösterilip gösterilmediğini, sütunların ne kadar geniş olduğunu, düzenleyicinin nasıl bölündüğünü, her düzenleyicinin hangi görünümü kullandığını ve binder'ın hangi düzenleyiciye yüklediğini. Belgeler hakkında hiçbir şey söylemez, bu nedenle her yerleşim her projede çalışır.

Bir yerleşimi **Görünüm > Yerleşim** altından ya da araç çubuğunun **Görünüm** düğmesinden seçin. Yedi yerleşim yerleşiktir:

| Yerleşim | Düzen |
|--------|-------------|
| **Varsayılan** | Binder, bir düzenleyici, denetçi |
| **Binder ve Düzenleyici** | Binder ve metin, denetçi yok |
| **Yalnızca Düzenleyici** | Yalnızca metin ve iki çubuk |
| **Anahat ve Düzenleyici** | Solda anahat, sağda metin. Binder grupları anahata yükler; bir satıra tıklamak o belgeyi sağda açar |
| **Mantar Pano ve Düzenleyici** | Üstte mantar pano, altta metin, aynı şekilde bağlı |
| **İki Düzenleyici** | Yan yana iki metin |
| **Planlama** | Binder, mantar pano ve özet ile notlarla denetçi |

**Görünüm > Yerleşim > Geçerli Yerleşimi Kaydet**, pencereyi şu anki haliyle seçtiğiniz bir adla saklar; yerleşimin pencerenin boyutunu da değiştirmesini istiyorsanız "Also remember the size of the window" kutusunu işaretleyin. **Yerleşimleri Yönet** tüm yerleşimleri küçük bir resimle gösterir ve kendi yerleşimlerinizi kullanmanıza, güncellemenize (geçerli pencereyle değiştirmenize), yeniden adlandırmanıza, silmenize, dışa ve içe aktarmanıza olanak tanır. Yerleşik yerleşimler değiştirilemez; bunun yerine değiştirilmiş pencereyi yeni bir adla kaydedin. Her yerleşimin bir klavye kısayolu olabilir (bölüm 19, "Layouts" grubu).

### Araç çubukları ve yerleri

0.91'den beri pencerede iki satırda sekiz araç çubuğu vardır (0.95'ten beri on; ikisi başlangıçta kapalıdır). İlk satır **Ana** (Görünüm, yeni metin, yeni klasör, çöp kutusu, Geri Al, Yinele), **Temalar** (tema düğmesi, başlangıçta kapalı), **Profiller** (kullanımdaki profil ve onu değiştirmek için bir menü, başlangıçta kapalı; 0.95'ten beri), **Arama** (arama düğmesi, hızlı arama alanı, görünüm modları) ve **Araçlar** (yer imleri, Hızlı Başvuru, yorumlar, Derle, Görünmeyen Karakterleri Göster, Yazım Modu, yazım araçlarının düğmeleri, denetçi) çubuklarını içerir. İkinci satır **Stil ve Yazı Tipi**, **Karakter ve Hizalama**, **Paragraf ve Ekleme**, **simge çubuğu** ve **Etiketler** (açık projenin her etiket rengi için bir düğme ve **Etiket Yok**; başlangıçta kapalı; 0.95'ten beri) çubuklarını içerir.

- **Bir araç çubuğunu taşımak.** Her araç çubuğunun önünde bir tutamaç vardır (iki sütun nokta; işaretçi üzerinde oklu bir artıya dönüşür). Tutamacı sürükleyin; araç çubuğu götürdüğünüz yere gider:
  - **Satırı boyunca.** İşaretçi iki komşu araç çubuğunun orta noktaları arasında kaldığı sürece, araç çubuğu onu hemen izler. Bir araç çubuğunun önündeki boş alan sizin yaptığınız kadar geniştir. Sağa giderken araç çubuğu önce arkasındaki boş alanı alır; bir sonraki araç çubuğuna ulaştığında, ondan sonraki araç çubukları satırın sonunda bir arada durabildiği ölçüde, onu da önüne katar. Sola giderken, önündeki araç çubuğunun bittiği yerde durur ve arkasındaki araç çubukları oldukları yerde kalır (yeniden bir arada olmalarını istiyorsanız onları ona kadar sürükleyin). Bir satırın ilk araç çubuğu pencerenin kenarında kalır.
  - **Sıralamada başka bir yere.** İşaretçiyi komşu bir araç çubuğunun ortasının ötesine ya da satırdaki başka herhangi bir araç çubuğunun üstüne götürün. Araç çubuğu şimdilik olduğu yerde kalır ve vurgu renginde bir çizgi nereye konacağını gösterir; oraya koymak için fare düğmesini bırakın. Böylece her sıralama mümkündür, bir satırın ilk ve son araç çubuğu için de.
  - **Başka bir satıra.** İşaretçiyi öbür satıra götürün: çizgi orada yeri gösterir. Her araç çubuğu her satırda durabilir: ana araç çubuğunun yanında bir biçim araç çubuğu ya da biçim araç çubuklarının arasında arama araç çubuğu.
  - **Kendi satırına ya da alt kenara.** Yeni bir satır açmak için işaretçiyi araç çubuğu satırlarının hemen altına (pencere boyunca bir çizgi belirir), araç çubuğunu oraya koymak için pencerenin alt kenarına götürün. Alt kenar, üst kenar gibi araç çubuklarını yan yana ve satırlar halinde alır.
  - **Esc** ya da hiçbir çizginin görünmediği bir yerde (örneğin pencerenin ortasında) bırakmak her şeyi olduğu gibi bırakır.

  Eski yerin ardındaki araç çubukları yaklaşır ve araç çubuğunun önündeki boş alan satırında kalır; yeni yerinde araç çubuğu, kendisinden önceki araç çubuğunun hemen arkasında durur ve oradan onu daha da sağa çekebilirsiniz. Bıraktığınız boş alan için fazla dar bir pencerede, bir araç çubuğu düğmelerini kaybetmeden önce boşluk küçülür; pencere yeniden genişleyince geri gelir. Kapatılmış bir araç çubuğu yerini korur. Bir araç çubuğu kenarlarda duramaz ya da kendi penceresi olarak yüzemez. {{APP}} satırları, sıralamayı ve boşlukları hatırlar.
- **Bir araç çubuğunu kapatmak.** **Görünüm > Araç Çubukları**, her araç çubuğu için bir anahtar içerir; aynı liste bir araç çubuğuna ya da araç çubuklarının yanındaki boş alana sağ tıklamakla da gelir. **Araç Çubuğunu Göster** ve **Biçim Çubuğunu Göster** eskisi gibi bütün bir satırı açıp kapatır (ilk satırın dört çubuğu, üç biçim araç çubuğu) ve yerleşimler bu ikisini kullanır; satırı kapalıyken tek bir çubuğun anahtarı kullanılamaz.
- **Görünüm > Araç Çubukları > Araç Çubuğu Yerlerini Sıfırla**, tüm araç çubuklarını standart yerlerine geri koyar (iki satır standart sıralarında, alt kenarda hiçbir şey yok) ve aralarındaki boşlukları kapatır.

### Kendi araç çubuklarınız

**Görünüm > Araç Çubukları > Araç Çubuklarını Özelleştir** (ya da bir çubuğa sağ tık) iki listeli bir pencere açar: solda kullanılabilir komutlar, sağda çubuğun içeriği. Üstten araç çubuğunu seçin.

- Bir komutu ok düğmesiyle ya da çift tıklayarak ekleyin. Menülerdeki her komut kullanılabilir, yazım araçlarının komutları da (Düzenle > Yazım Araçları); listenin üstündeki arama alanı onu bulur.
- Bir düğmeyi diğer okla ya da çift tıklayarak kaldırın, yukarı ve aşağı düğmeleriyle ya da sürükleyerek taşıyın.
- **Ayırıcı** ve **Esnek Boşluk** istediğiniz kadar kullanılabilir. Esnek bir boşluk, arkasından gelenleri sağa iter; iki tanesi aralarında duranı ortalar.
- Bazı girişler birden çok komuttur: ilk satırın dört araç çubuğu için **Görünüm**, **Ekle**, **Arama**, **Hızlı Arama alanı**, **Görünüm Modu**, **Proje Yer İmleri**, **Yorumlar ve Dipnotlar**, **Yerleşimler** ve **Temalar** (menülü tema düğmesi); üç biçim araç çubuğu için stil, yazı tipi, boyut ve satır aralığı listeleri ve dört paragraf düğmesi. Böyle bir denetim bir tanedir: bir araç çubuğuna eklendiğinde, onu taşıyan araç çubuğundan oraya geçer. İlk satırın bir denetimi bir biçim araç çubuğuna konamaz, tersi de öyle.
- Listelerin altında küçük, orta ya da büyük simgeleri ve ilk satırın araç çubuklarının metni simgelerin yanında mı yoksa altında mı göstereceğini seçersiniz.
- **Standart Küme**, programın geldiği düğmeleri çubuğa geri koyar.

Bir çubuk pencereye sığandan fazlasını içeriyorsa, ucundaki bir ok geri kalanı açar. Hiçbir düğmesi kalmayan bir araç çubuğu kapatılır; **Görünüm > Araç Çubukları** altından yeniden açıldığında standart kümesine sahiptir. 0.91'den önceki bir sürümde düzenlediğiniz bir araç çubuğu yeni araç çubukları arasında bölündü: her düğme, onu standart kümelerde tutan araç çubuğuna gitti ve çıkardığınız şeyler çıkarılmış kalır.

### Simge çubuğu

**Simge çubuğu**, sık kullandığınız en fazla on belge simgesini içerir. Bunlardan birine tıklamak, o simgeyi seçili belgelere ve klasörlere verir (binder'da ya da üzerinde çalıştığınız mantar panoda ya da anahatta); onlardan sonraki **↺** düğmesi onlara standart simgelerini geri verir ve son düğme çubuğun simgelerini seçer. Çubuğu **Görünüm > Araç Çubukları > Simge Çubuğu** ile gösterin ya da gizleyin.

**Görünüm > Araç Çubukları > Simge Çubuğu İçin Simgeleri Seç…** (çubuğun sağ tık menüsünde de) solda tüm simgeleri, kendi proje simgeleriniz dahil, sağda çubuğun simgelerini içeren bir pencere açar. **Add →** ile ya da çift tıklayarak ekleyin, **← Remove** ile ya da çift tıklayarak kaldırın ve sırayı **Up** ve **Down** ile değiştirin. **Standart Küme**, programın geldiği on simgeyi geri koyar (Green Book, Proofread, Ticked, Unticked, Question Mark, Red Flag, Yellow Notepad, Character Sheet, Location Sheet, Warning).

---

## 3. Binder

Her projede üç özel klasör vardır:

- **Draft** (bazı şablonlarda *Manuscript*): kitabınız olacak her şey. Yalnızca Draft'ın içindekiler derlenir.
- **Research**: notlar, karakter sayfaları, resimler, PDF'ler ve topladığınız diğer her şey.
- **Trash**: silinen öğeler, **Proje > Çöp Kutusunu Boşalt** seçene kadar burada bekler. Sorusu, geri alınamayacak bir şeyden önce gelen her soru gibi, düğmesinde adımı adlandırır ve varsayılan olarak **İptal** düğmesine sahiptir; böylece Enter tuşu hiçbir şeyi asla silmez (0.70).

### Öğelerle çalışmak

- **Yeni belge**: **Ctrl+N** (ya da ➕ düğmesi). Seçili öğenin altına eklenir. "Untitled" adlı (ya da başlıksız) bir belge, yazar yazmaz metninin ilk satırını başlık olarak alır: en fazla 60 karakter, bir sözcükte kesilmiş, satır içi açıklamalar, satır içi dipnotlar ve resimler olmadan. Başlık, belgeyi kendiniz yeniden adlandırana kadar ilk satırı izler; metni silinen bir belge yeniden "Untitled" olur. Zaten metni olan "Untitled" belgeler proje açıldığında adlarını alır. Başlık sıradan bir başlıktır: kaydedilir, derlenir (başlıklar derlendiğinde, başlık olan bir ilk satır iki kez görünür) ve Scrivener projelerine yazılır.
- **Yeni klasör**: **Alt+Shift+N**.
- **Yeniden adlandırma**: **F2** ya da başlığa yavaşça çift tıklama.
- **Taşıma**: öğeleri fareyle sürükleyin ya da **Ctrl+Alt+Ok tuşları** kullanın: Yukarı ve Aşağı klasör içinde taşır; Sol klasörün dışına taşır; Sağ üstteki öğenin içine taşır. İki öğenin *arasına* bırakılan bir belge oraya gider; bir öğenin *üzerine* bırakılırsa onun alt belgesi olur. Belgeleri yanlışlıkla iç içe geçiriyorsanız, **Dosya > Seçenekler > Davranışlar** altında metin belgelerinin üzerine bırakmayı kapatın (klasörler her zaman bırakılanları kabul eder).
- **Bütün bir dalı açmak ya da kapatmak**: **Alt** tuşunu basılı tutun ve bir klasörün ya da grubun okuna tıklayın.
- **Silme**: **Delete** tuşuna basın. Öğe Çöp Kutusuna gider; onu seçilen birkaç belgeden biri olarak gösteren bir mantar pano ya da anahat onu hemen bırakır (0.94.0).
- **Çoğaltma**: **Ctrl+D**.
- **Bölme ve birleştirme**: **Belgeler > Böl > Seçimden** (**Ctrl+K**) belgeyi imlecin olduğu yerden ikiye keser; bir tablonun ya da bir resmin açıklamasının içinde çalışmaz (imleci önüne ya da arkasına koyun). **Belgeler > Birleştir** (**Ctrl+Shift+M**) seçili belgeleri, özetleri, anahtar sözcükleri ve notlarıyla birlikte ilkine katar; diğerleri Çöp Kutusuna gider.
- **Simgeyi değiştirme**: sağ tık > **Simgeyi Değiştir** (ya da **Belgeler > Simgeyi Değiştir**) ya da simge çubuğu (bölüm 2). Aşağıdaki "Simgeler" bölümüne bakın.

**Sağ tık.** Bir belgede sağ tık menüsü, seçili belgeler için komutları içerir (yeniden adlandırma, çoğaltma, Çöp Kutusuna taşıma, anlık görüntüler, etiket, durum ve daha fazlası). Belgelerin altındaki boş alanda yalnızca yeni bir şey yapanları sunar: **Yeni Metin**, **Yeni Klasör**, **Şablondan Yeni** ve binder'ın **Dış Görünümü**.

Bir belgenin, tıpkı bir klasör gibi alt belgeleri olabilir. Klasörler ve belgeler aynı şekilde davranır; klasörler yalnızca farklı görünür ve varsayılan olarak mantar pano olarak açılır.

**Binder'ın tuşları (0.94.0).** Klavye binder'dayken **Return** seçili belgeyi düzenleyicide açar ve klavyeyi oraya koyar (bir dosya yöneticisinin yaptığı gibi) ve ok tuşları listede gezinir. **Space** bir adı yazmanın parçasıdır: bir başlığın ilk harflerini, boşluklar dahil ("Chapter t") yazın; seçim böyle başlayan ilk başlığa atlar. **Görünüm > Binder'da Hızlı Ekleme Tuşları** girişi başlangıçta kapalıdır (bir güncellemeden sonra da). Açıldığında binder hızlı ekleme tuşlarına sahip olur: **Return** seçili belgenin altına yeni bir boş belge yapar ve **Space** seçili belgeleri Hızlı Başvuru panelleri olarak açar (ve bir koleksiyonun ya da aramanın listesinde).

Binder'da bir tıklama belgeyi düzenleyiciye yükler. **Geri**'den, bir belge bağlantısından sonra ya da kilitli bir düzenleyicide, düzenleyici binder'da seçilenden başka bir belgeyi gösterebilir: o zaman bir **gri çubuk** binder'da düzenleyicinin belgesini işaretler. **Gezin > Binder'da Göster** (**Ctrl+Shift+R**) onu seçer.

**Dar binder, derin klasörler.** Klasörlerin her düzeyi 12 px girintilidir (eskisinin yarısı; **Dosya > Seçenekler > Dış Görünüm > Binder > Düzey başına girinti** altında değiştirin). Başlıklar binder'ın genişliğinden uzun olduğunda, binder'ın altında bir kaydırma çubuğu belirir; böylece binder'ı genişletmeden uzun başlıkların sonunu okuyabilirsiniz.

### Simgeler

**Belgeler > Simgeyi Değiştir** (binder'ın, mantar panonun ve anahatın sağ tık menüsünde de), Windows için Scrivener 3'ün simge menüsü gibi kurulmuştur:

- **Reset Icon to Default**: bir belgenin, klasörün ya da dosyanın standart simgesi geri gelir.
- **Icon from Text…**: en fazla üç karakterli bir simge yapar (örneğin *PR*, *Ch1* ya da *A*). Kutunun, harflerin ve kenarlığın rengini seçin; küçük resim sonucu gösterir. Simge projede saklanır ve seçili belgelere hemen verilir.
- **Manage Icons…**: bu projenin simgeleri. **Add Picture…** resimleri (PNG, SVG, JPEG ve diğerleri) simgelere dönüştürür, **From Text…** yukarıdakiyle aynıdır, **Rename…** ve **Delete** listeyi değiştirir (simgesi silinen belgeler standart simgelerini geri alır), **Use for Selected Documents** seçilen simgeyi seçili belgelere verir. Simgeler projede saklanır (*icons* klasörü), böylece onunla birlikte taşınır.
- **Recent**: seçtiğiniz son beş simge.
- **Book**, **Characters**, **Flag**, **Locations**, **Notes**, **To Do**, **TV**, **Way-Station**: her grubun simgelerini birkaç renkte içeren alt menüler (örneğin Mavi, Yeşil, Kırmızı ve Sarı Kitap; yedi bayrak; beş not defteri; To Do'da Ticked ve Unticked, Pass, Fail ve Unclear).
- Sonra tek simgeler: Back Matter, Blackboard, Calendar, Clapper Board, Clock, Cog, Eye, Figures and Graphs, Filing Cabinet, Front Matter, Information, Lectern, Light Bulb, Magnifying Glass, Music, Musical Notes, Outline, Pencil, Proofread, Question Mark, Reviewer Comments, Scissors, Speech Bubble, Speech Marks, Statistics, Sync Folder, Tables, Test Tube, Thought Bubble ve Warning.
- **Project Icons**: kendi simgeleriniz (projede varsa) ve **More Icons (Desktop)**: önceki sürümlerin sunduğu masaüstü temasının simgeleri.

**Simgelerde etiket rengi.** Birçok simgenin beyaz kısımları (bir not defterinin sayfaları, beyaz bir bayrağın kumaşı, bir sayfanın kâğıdı, metinden yapılmış bir simgenin kutusu) belgenin etiket rengini alır. Bunu **Görünüm > Etiket Rengini Şurada Kullan > Simgeler** ile (ya da **Dosya > Seçenekler > Dış Görünüm > Binder**) açın ya da kapatın.

**Simgeler ve Scrivener.** {{APP}}'in simgeleri yeniden çizilmiştir, ancak her biri karşılığı olduğu Scrivener simgesinin adını taşır. Scrivener biçimindeki bir proje (bölüm 14) bu adları korur, böylece Scrivener onlar için kendi simgesini gösterir; Scrivener'da yapılan simgeler buraya eşleşen simge olarak geri gelir. Kendi simgeleriniz *.scriv* içindeki *Icons* klasörüne gider. Bu programın resmi olmadığı bir simge adını korur ve burada en yakın simge olarak gösterilir.

### Etiketler, durum ve "Derlemeye Dahil Et"

Bir öğenin **etiketini** (bir renk, örneğin bakış açısı karakteri için) ve **durumunu** (örneğin *First Draft* ya da *Done*) ayarlamak için ona sağ tıklayın. Listeleri **Proje > Proje Ayarları** altında değiştirin.

Etiketli bir belge ya da klasör, rengini denetçide de gösterir: özet kartı ve belge notları etiket renginin açık bir tonunu alır (doygunluğunun %15'i), böylece öğenin hangi etikete sahip olduğunu bir bakışta görürsünüz. Etiketsiz ya da gri etiketli öğeler normal kart rengini korur.

**Etiketler** araç çubuğu (**Görünüm > Araç Çubukları > Etiketler**, başlangıçta kapalı) açık projenin tüm etiket renklerini yan yana gösterir ve etiketsiz bırakmak için bir düğme içerir. Bir tıklama, **Belgeler > Etiket** gibi, seçili belgelerin etiketini hemen ayarlar; her rengin ipucu etiketin adını söyler. Yeni bir projenin başlangıçta sahip olduğu etiketler **Dosya > Seçenekler > Dış Görünüm > Standart Renkler** altında ayarlanır.

**Derlemeye Dahil Et** kapalı olan öğeler Draft içinde kalır, ancak derlediğinizde dışarıda bırakılır.

### Koleksiyonlar

Koleksiyonlar, örneğin "Needs research" ya da "Scenes with Anna" gibi ek belge listeleridir. Onları **Koleksiyonlar** düğmesiyle gösterin. Seçili belgelerden bir koleksiyon oluşturmak için binder'ın üstündeki ➕ düğmesini kullanın ya da belgelere sağ tıklayıp **Add to Collection** seçin. Bir belgeyi koleksiyondan çıkarmak onu silmez. Kayıtlı aramalar (bölüm 9) da koleksiyonlardır; kendilerini güncellerler.

- **Belge eklemek** için onları binder'dan (ya da bir düzenleyicinin başlık simgesinden, bir mantar panodan, bir anahattan) koleksiyonun sekmesine sürükleyin.
- Bir koleksiyonun **listesinin içinde**, belgeleri istediğiniz sıraya sürükleyin (binder sırası değişmez), belgeleri bir yere bırakın ve seçili belgeleri koleksiyondan çıkarmak için **Delete** tuşuna basın. Arama koleksiyonları bu şekilde değiştirilemez.
- Koleksiyonları başka bir sıraya koymak için **sekmeleri sürükleyin**.
- Listedeki belgeler binder'dakiler gibi sürüklenebilir ve **Space** seçilenleri Hızlı Başvuru panelleri olarak açar.

---

## 4. Yazım

### Metin biçimlendirme

| İşlem | Kısayol |
|--------|----------|
| Kalın / italik / altı çizili | Ctrl+B / Ctrl+I / Ctrl+U |
| Vurgula | Ctrl+Shift+H |
| Üst simge / alt simge | Ctrl+Shift++ / Ctrl+= (Almanca düzen: alt simge için Ctrl+Shift+D, aşağıdaki "Kısayollar ve klavye düzeni" bölümüne bakın) |
| Sola / ortaya / sağa hizala | Ctrl+{ / Ctrl+\| / Ctrl+} (Almanca düzen: Alt+Shift+L / Alt+Shift+E / Alt+Shift+R) |
| Yazı tipleri penceresi | Ctrl+T |
| Yazı tipi boyutunu büyüt / küçült | Ctrl+> / Ctrl+< |
| Paragraf penceresi | Ctrl+Alt+P |
| Biçimlendirmeyi kopyala / yapıştır | Ctrl+Alt+C / Ctrl+Alt+V |
| Yakınlaştır / uzaklaştır / yakınlaştırmayı sıfırla | Ctrl++ / Ctrl+- / Ctrl+0 |
| Farenin altındaki alanı yakınlaştır | Ctrl+fare tekerleği |
| Cetveli göster veya gizle | Ctrl+Alt+R |
| Biçimlendirmesiz yapıştır | Ctrl+Shift+V |
| Biçimlendirmeyle yapıştır (diğer programlardan) | Ctrl+Alt+Shift+V |
| Geri al / yinele | Ctrl+Z / Ctrl+Shift+Z (aşağıdaki "Geri alma ve yineleme" bölümüne bakın) |

Kalın, italik, üst simge ve benzerleri seçim üzerinde çalışır. Seçim yoksa, bir sözcüğün içinde bütün sözcüğü değiştirirler; bir sözcüğün sonunda (ya da imlecin çevresinde hiçbir şey yoksa) bir sonraki yazdığınız şeyi değiştirirler: H₂O için "H" yazın, Ctrl+= tuşlarına basın, "2" yazın, yeniden Ctrl+= tuşlarına basın ve "O" yazın.

**Stiller** (Başlık 1–6, Ana Başlık, Alt Başlık, Blok Alıntı ve kendi stilleriniz) biçim çubuğunun solundaki listeden seçilir. Stiller Derle'ye bir paragrafın ne olduğunu söyler; bu nedenle metni elle büyük ve kalın yapmak yerine onları kullanın. Aşağıdaki "Stiller" bölümüne bakın.

**Biçim çubuğunun gösterdiği.** Çubuk, metni seçimin tamamı için gördüğünüz gibi gösterir. **Kalın**, **İtalik**, **Altı Çizili**, **Üstü Çizili**, üst simge ve alt simge, seçili metnin tamamı öyle göründüğünde açıktır, stili onu öyle yaptığında da (bir başlığın kalını). Bir tıklama gördüğünüz şeyi tersine çevirir: kalın bir başlıkta **Kalın**, onu kalın olmaktan çıkarır ve proje kaydedilip yeniden açıldığında da kalın olmaz. Yazı tipi, boyut, satır aralığı, hizalama düğmeleri ve stil listesi de seçimi gösterir; seçim iki yazı tipi, iki boyut, iki hizalama ya da iki stilde paragraflar içerdiğinde o alan boş kalır (hiçbir hizalama düğmesi açık olmaz). Boş alana yazılan bir boyut, seçimin tamamına o boyutu verir.

**Listeler**: liste düğmelerini kullanın ya da bir paragrafın başına "- " veya "1. " yazın.

**Akıllı tipografi**: düz tırnaklar kıvrık tırnaklara dönüşür ve yazarken iki kısa çizgi bir uzun çizgiye dönüşür. Bunu **Dosya > Seçenekler > Düzenleme** altında kapatın.

**Revizyon modu** (**Biçim > Revizyon Modu**): yazdığınız her şey beş revizyon renginden biriyle renklendirilir; böylece bir düzenleme turunda neyin değiştiğini görebilirsiniz. Durdurmak için **Kapalı**'yı seçin.

**Biçim çubuğunda aralıklar ve girintiler.** Liste düğmelerinden sonraki dört düğme, imlecin ya da seçimin bulunduğu paragrafları ayarlar: paragrafın **öncesindeki boşluk** ve **sonrasındaki boşluk** (0, 3, 6, 9, 12 ya da 18 pt), **ilk satırın girintisi** ve **bütün paragrafın girintisi** (cetvelin gösterdiği gibi santimetre ya da inç cinsinden). İmlecin bulunduğu paragrafın değeri işaretlidir; **Diğer...** herhangi bir değer sorar. **Düzenle > Geri Al** bir değişikliği geri alır. Kendiniz düzenlediğiniz bir biçim çubuğu bu dört düğmeyi liste düğmelerinden sonra bir kez alır; **Görünüm > Araç Çubukları > Araç Çubuklarını Özelleştir** onları taşır ya da kaldırır.

**Geri alma ve yineleme.** **Düzenle > Geri Al** ve **Düzenle > Yinele**, klavyenin bulunduğu metin alanında (düzenleyici, özet, notlar, bir başlık) çalışır ve neyi geri alacaklarını söyler: "Yazmayı Geri Al", "Yapıştırmayı Geri Al", "Kalını Geri Al", "Silmeyi Yinele". Klavye başka bir yerdeyse, örneğin binder'da, grileşirler: belgeleri Çöp Kutusuna taşıma, Çöp Kutusunu boşaltma ve anlık görüntüleri silme bu şekilde geri alınamaz (Çöp Kutusundaki bir belge dışarı sürüklenerek geri gelir; anlık görüntüler için bölüm 11'e bakın). Araç çubuğunun **Geri Al** ve **Yinele** düğmelerinin yanındaki ok, adımları, sıradaki ilk olacak şekilde listeler; birini seçmek o adımı ve listede ondan önceki her adımı geri alır (ya da geri getirir). Biçim menüsünün her komutu bir adımdır.

**Biçim çubuğunda metin rengi ve vurgu.** Renkli çubuklu iki düğme (vurgulamak için bir kalem, metin rengi için bir *T* harfi) Scrivener'ınkiler ve Word'ünkiler gibi çalışır: bir tıklama çubukta gösterilen rengi seçili metne ya da seçim yoksa bir sonraki yazdığınız şeye uygular. Bir düğmenin üzerinde bir an bekleyin; altında **kırmızı, mavi, sarı, yeşil, beyaz ve siyah** renkleriyle, ayrıca **Vurgusuz** (metin rengi için **Otomatik**) ve herhangi bir başka renk için **…** içeren küçük bir pencere açılır. Bir renge tıklamak onu uygular ve düğmenin rengi yapar; düğme onu bir sonraki tıklama için korur (yeniden başlatmadan sonra da). Bir vurgu, açık ya da koyu sayfada fark etmeksizin tam olarak seçtiğiniz renkle çizilir. Üzerindeki metnin okunmasının zor olacağı yerde (4,5 : 1 karşıtlığın altında) metin yalnızca ekranda koyu ya da açık gösterilir; saklanan rengi ve kaydedilmiş ve derlenmiş dosyalardaki vurgu seçtiğiniz gibi kalır.

### Yazı tipleri

Bir projenin tüm metni tek bir görünüme sahiptir: **varsayılan biçimlendirme** (aşağıya bakın). Bunun üzerine herhangi bir metin parçasına kendi yazı tipini verebilirsiniz.

- **Biçim çubuğu**: yazı tipi listesi ve boyut alanı seçili metni hemen değiştirir. Seçim yoksa, imlecin bulunduğu **bütün paragrafı** (ve orada bir sonraki yazdığınız şeyi) değiştirirler. Boyut alanının okları metni her tıklamada değiştirir; yazdığınız boyut **Enter** ile geçerli olur. Biçim çubuğu, Biçim menüsü ve kısayolları imlecin en son bulunduğu metne etki eder: bir düzenleyici, denetçideki notlar, denetçideki yer imli metin ya da bir nesnenin uzun bir metin alanı (orada: kalın, italik, altı çizili, üstü çizili, metin rengi ve vurgu). Boyut alanına bir tıklama bunu değiştirmez.
- Biçim çubuğunda bir değişiklikten sonra, düzenleyicinin üstündeki bir çubuk aynısını bütün proje için önerir: stili olan bir paragrafta (örneğin bir başlık) **“Başlık 1” Stilini Değiştir** stile yeni yazı tipini ya da boyutu verir ve her belgede o stildeki tüm metin onu izler; stili olmayan bir paragrafta **Tüm Metin İçin Kullan** onu proje yazı tipi ya da boyutu yapar (aşağıdaki "Varsayılan biçimlendirme" bölümüne bakın). Değişikliği olduğu yerde bırakmak için **×** düğmesine tıklayın ya da başka bir yerde yazmaya devam edin. Değişen her belge önce bir anlık görüntü alır.
- **Biçim > Yazı Tipi > Yazı Tiplerini Göster…** (**Ctrl+T**) yazı tipleri penceresini açar: yazı tipi, yazı stili (ince, normal, kalın, italik ve yazı tipinin sunduğu diğer her şey), boyut, altı çizili, üstü çizili, büyük harfler (küçük büyük harfler ya da tümü büyük harf), yüzde olarak harf aralığı ve metin rengi. Yalnızca pencerede değiştirdiğiniz şey uygulanır, böylece kalın ve düz sözcükler içeren bir seçim onları korur. **Proje Yazı Tipi** metni projenin yazı tipine, boyutuna ve rengine geri döndürür.
- **Kalın, İtalik, Altı Çizili, Üstü Çizili**: seçili metnin *tamamı* o biçime sahipse, nasıl seçtiğinizden bağımsız olarak (sağdan sola da) bir düğme açıktır. Yalnızca kısmen kalın olan bir seçim Kalın'ı kapalı gösterir ve **Kalın** o zaman hepsini kalın yapar; ikinci bir tıklama hepsini düz yapar. Yazı tipleri penceresinde de seçtiğiniz bir yazı stili aynı şekilde seçimin tamamına uygulanır.
- **Biçim > Yazı Tipi > Büyüt / Küçült** (**Ctrl+>** / **Ctrl+<**) boyutu bir punto değiştirir; seçimin her parçası kendi boyutunu korur ve büyür ya da küçülür.
- **Biçim > Yazı Tipi > Metin Rengi…** ve **Otomatik Metin Rengi**.

Projenin kendi yazı tipini ya da boyutunu seçmek "kendi yazı tipi yok" olarak saklanır: böyle bir metin, varsayılan biçimlendirmeyi daha sonra değiştirdiğinizde projeyi izler.

**Birkaç tasarımlı yazı tipleri.** Bazı yazı tipleri bir yazı stilinin birkaç tasarımını tek bir ad altında tutar: örneğin Windows 11'in Sitka'sı optik boyutları Small, Text, Subheading, Heading, Display ve Banner'ı içerir (her biri kendi boyut aralığı için çizilmiştir, büyükler daha dar ve daha küçük küçük harflidir) ve bazı yazı tipleri aynı şekilde dar ya da geniş sürümler içerir. {{APP}} her tasarımı kendi yazı tipi olarak listeler (örneğin **Sitka Small**, **Sitka Text**, **Sitka Banner**), yalnızca o tasarımın yazı stilleriyle; böylece kalın ve italik seçtiğiniz tasarımın içinde kalır; 0.40.1'den önce kalın metin başka bir tasarımda çıkabilir ve daha küçük ve daha dar görünebilirdi. Yazı tipi dosyaları değiştirilmez. Özgün adlı yazı tipi kendi tasarımını korur, böylece onunla daha önce yazılan metin görünümünü korur.

**Yazı tipi kurmak.** **Biçim > Yazı Tipi**, bilgisayarınızda henüz olmayan yazı tipleri için komutlar içerir:

- **Yazı Tipi Dosyalarını Kur…**: yazı tipi dosyalarını (TTF, OTF, TTC, PFB, WOFF ya da onları içeren bir ZIP dosyası) seçin. Kendi yazı tipi klasörünüze (*~/.local/share/fonts*) kopyalanırlar ve {{APP}}'in yazı tipi listeleri onları hemen tanır; diğer programlar onları bir sonraki başlatmalarından sonra görür. Parola gerekmez ve yazı tipleri yalnızca kullanıcınız içindir.
- **Yazı Tipi Klasörünü Aç**: o klasörü dosya yöneticisinde açar; oraya kendiniz koyduğunuz yazı tipleri **Yazı Tipi Listesini Yenile**'ten sonra bulunur.
- **Masaüstünün Yazı Tipi Yöneticisini Aç**: KDE'nin yazı tipi yöneticisini (Sistem Ayarları > Metin ve Yazı Tipleri > Yazı Tipi Yönetimi) ya da başka bir masaüstünün yazı tipi programını açar; orada tüm kullanıcılar için yazı tipi de kurabilir ve yazı tiplerini kaldırabilirsiniz.
- **Yazı Tipi Listesini Yenile**: yeni yazı tiplerini arar ve adlandırır.

### Paragraf biçimlendirme

**Biçim > Paragraf > Paragraf…** (**Ctrl+Alt+P**), imlecin ya da seçimin bulunduğu paragraflar için şunları ayarlar: hizalama, sol ve sağ girinti, ilk satır girintisi (negatif bir değer asılı girinti verir), öncesindeki ve sonrasındaki boşluk, satır aralığı (satır yüksekliğinin bir katı, kesin bir yükseklik ya da en az bir yükseklik) ve sekme durakları (sol, ortalanmış, sağ, ondalık noktası). Yalnızca değiştirdiğiniz şey uygulanır. **Reset to Default** elle ayarladığınız biçimlendirmeyi kaldırır; paragraf yeniden stilini ve projeyi izler.

Hızlı yollar: hizalama düğmeleri, biçim çubuğundaki satır aralığı listesi, **Biçim > Paragraf > Satır Aralığı** ve **Girintiyi Artır / Girintiyi Azalt**.

Bir paragraf stili uygulamak, ulaştığı paragraflardaki elle ayarlanmış biçimlendirmenin yerini alır: girintiler, aralıklar, hizalama ve sekme durakları, ve içlerindeki metnin yazı tipleri, boyutları, renkleri, harf aralığı ve büyük harfleri. Kalın, italik, altı çizili, üstü çizili, vurgular ve karakter stilleri kalır. Birkaç paragraf boyunca metin seçerek hepsine stili verin, her paragrafın tamamına; bir paragrafın başında biten bir seçim (örneğin **Shift+Aşağı** tuşlarından sonra) o paragrafı olduğu gibi bırakır.

**Biçim > Biçimlendirmeyi Kopyala** (**Ctrl+Alt+C**) imlecin bulunduğu yerdeki yazı tipini ve paragraf biçimlendirmesini hatırlar; **Biçimlendirmeyi Yapıştır** (**Ctrl+Alt+V**) onu seçime ya da hiçbir şey seçili değilse bütün paragrafa uygular.

### Görünmeyen karakterler

**Görünüm > Metin Düzenleme > Görünmeyen Karakterleri Göster** (araç çubuğunun ¶ düğmesi de) görmediğiniz karakterleri gösterir: her boşluk için bir nokta, bir sekme için bir ok, bölünmeyen bir boşluk için küçük bir daire, bir satır sonu için (**Shift+Enter**) bükülmüş bir ok ve her paragrafın sonunda ¶. Yalnızca ekranda, her düzenleyicinin ana metninde çizilirler; asla metnin, bir baskının ya da derlemenin parçası olmazlar. Seçim saklanır. Renkleri metnin rengidir, kâğıda doğru soluklaştırılmış; kendi renkleri **Dosya > Seçenekler > Dış Görünüm > Metin İşaretleri** altında (ya da metne sağ tık > **Dış Görünüm**) ayarlanır ve kâğıt ve metin gibi projenin görünümüne aittir (yazım modu soluklaştırılmış metin rengini korur).

### Cetvel

**Görünüm > Metin Düzenleme > Cetveli Göster** (**Ctrl+Alt+R**) metnin üstünde bir cetvel gösterir. İmlecin bulunduğu paragrafa aittir.

- Üstteki işaret **ilk satır girintisidir**, alttaki işaretler **sol girinti** ve **sağ girintidir**. Onları sürükleyin. Sol girinti ilk satırı da beraberinde götürür.
- Bir sekme durağı eklemek için cetvelin alt yarısına **tıklayın**. Bir sekme durağını taşımak için sürükleyin; kaldırmak için cetvelin dışına yukarı ya da aşağı sürükleyin. Sol, ortalanmış, sağ ya da ondalık yapmak için bir sekme durağına **sağ tıklayın**.
- İşaretler bir santimetrenin çeyreklerine (bir inçin on altıda birlerine) yapışır; onları serbestçe yerleştirmek için **Shift** tuşunu basılı tutun.
- Paragraf penceresi için cetvele **çift tıklayın**. Sağ tık menüsü santimetre ve inç arasında geçiş yapar (**Dosya > Seçenekler > Düzenleme** altında da).

Sekme durakları paragrafın ilk satırının başından ölçülür. Kendi sekme durakları olmayan bir paragrafta her 2,1 cm'de bir sekme konumu vardır.

Cetvel gerçek uzunlukları gösterir. Yakınlaştırma ekrandaki harfleri büyütür, ancak girintileri ve sekme duraklarını büyütmez; bu nedenle mesafeleri %100'de karşılaştırın.

### Varsayılan biçimlendirme

- **Bu proje**: **Proje > Proje Ayarları > Biçimlendirme** (biçim çubuğundaki sürgü düğmesi de) bütün projenin metninin yazı tipini, boyutunu, satır aralığını, ilk satır girintisini, paragraflardan sonraki boşluğu ve hizalamasını ayarlar (dört düğme: sol, ortalanmış, sağ, iki yana yaslı). Stiller Panelinde **Stil Yok** > **Seçimden Yeniden Tanımla** bu değerleri imlecin bulunduğu paragraftan alır (aşağıdaki "Stiller" bölümüne bakın). **Apply** tüm belgeleri bir kerede değiştirir ve pencereyi açık bırakır, böylece sonucu arkasında görebilirsiniz; **Tamam** aynısını yapar ve pencereyi kapatır. **Use Current** değerleri imlecin bulunduğu paragraftan alır.
  - **Change text with a font, size or spacing of its own as well** (varsayılan olarak açık) ile, elle biçimlendirilmiş ya da bir içe aktarmadan kendi yazı tipiyle gelen metin de, belgesinin ana yazı tipine, boyutuna ve aralığına sahip olduğu sürece izler. Ana metinden farklı olan olduğu gibi kalır: daha büyük bir başlık satırı, başka bir yazı tipinde bir bölüm, girintisiz bir paragraf. Elle biçimlendirilmiş tüm metni olduğu gibi bırakmak için kutuyu kapatın.
  - Değişen her belge önce bir anlık görüntü alır (denetçi, anlık görüntüler sekmesi, "Before the project formatting changed"), böylece geri dönebilirsiniz. Senaryo belgeleri kendi biçimlerini korur.
- **Yeni projeler**: **Dosya > Seçenekler > Düzenleme > Formatting of new projects**, aynı dört hizalama düğmesi ve **Use Formatting in Current Editor** ile. Mevcut projeler biçimlendirmelerini korur. Standart 11,0 pt, satır aralığı 1,09, ilk satır girintisi yok ve paragraflardan sonra 6 pt'dir. Scrivener'dan içe aktarılan projeler de bu değerlerle başlar (Scrivener projesinde en çok kullanılan yazı tipiyle).
- **Varsayılana dönmek**: binder'da belgeleri seçin ve **Belgeler > Dönüştür > Metni Varsayılan Biçimlendirmeye Döndür…** seçin. Yazı tiplerinin ve renklerin, girintilerin ve aralıkların, hizalamanın ve sekme duraklarının kaldırılıp kaldırılmayacağına siz karar verirsiniz. Kalın, italik, stiller, notlar ve bağlantılar her zaman kalır. Önce her belgenin bir anlık görüntüsü alınır (denetçi, anlık görüntüler sekmesi), böylece geri dönebilirsiniz.

### Stiller

Bir stil, biçimlendirmeyi bir ad altında saklar. Metin adı hatırlar; bu nedenle stili değiştirdiğinizde, onu kullanan tüm metin, bütün projede onunla birlikte değişir.

İki tür vardır:

- **Paragraf stilleri** bütün paragraflara uygulanır. Paragraf biçimlendirmesini (hizalama, girintiler, aralıklar, satır aralığı, sekme durakları) ve isterseniz paragraf için karakter biçimlendirmesini (yazı tipi, boyut, kalın, italik, renk) tutarlar. Listelerde "¶" ile, karakter biçimlendirmesi içerdiklerinde "¶a" ile işaretlenirler.
- **Karakter stilleri** seçili metne ya da imlecin bulunduğu sözcüğe uygulanır. Yalnızca karakter biçimlendirmesini tutarlar. "a" ile işaretlenirler.

Gövde metnini **Stil Yok** üzerinde bırakın. Projenin varsayılan biçimlendirmesini izler ve Derle ona istediği görünümü verebilir. Stilleri yalnızca gövde metninden farklı görünmesi gereken metin için kullanın.

**Bir stil uygulamak**: onu biçim çubuğunun solundaki listeden, **Biçim > Stil** altından, metindeki bir sağ tıkın menüsünden ya da metnin yanında açık kalan küçük bir pencere olan **Stiller Panelinden** (**Biçim > Stil > Stiller Paneli**) seçin; bir satıra çift tıklamak stili uygular. **Stil Yok**, seçimden bir karakter stilini kaldırır; hiç yoksa paragraf stilini kaldırır. Stiller Panelinin açtığı pencereler ve sorular panelin üstünde görünür.

Biçim çubuğunun ve sağ tık menüsünün **stil listesi** (**Stil**), seçtiğiniz stilleri seçtiğiniz sırayla gösterir: **Stil Listesini Özelleştir…** (listenin altında ve **Biçim > Stil** altında) tüm girişleri listeler; ihtiyacınız olmayanların işaretini kaldırın, onları başka bir sıraya sürükleyin ya da okları kullanın ve **All, in the Order of the Styles** tam listeyi geri getirir. On girişten fazlası olduğunda liste on tanesini gösterir ve kayar. Altında, her zaman görünür olarak: **Seçimden Yeni Stil…**, **Seçimden Stili Yeniden Tanımla…**, **Stiller Paneli** ve **Stil Listesini Özelleştir…**. Listede olmayan bir stil, metin onu kullandığında biçim çubuğunda yine de adlandırılır; **Biçim > Stil** her zaman tüm stilleri listeler. Seçim projeye aittir (yeni bir stil listenin sonunda görünür) ve proje ayarları dosyalarındaki stillerin parçasıdır.

**Stil Yok ana metin stilidir.** Stiller Panelinde **Stil Yok** üzerine sağ tıklayın (ya da seçin ve Yeniden Tanımla düğmesini kullanın) ve **Seçimden Yeniden Tanımla…** seçin: imlecin bulunduğu paragrafın yazı tipi, boyutu, satır aralığı, ilk satır girintisi, paragraflardan sonraki boşluğu ve hizalaması projenin ana metin stili olur (**Proje > Proje Ayarları > Biçimlendirme** değerleri). Bir soru önce değerleri gösterir. Bütün projede stili olmayan tüm metin onu izler, kendi yazı tipi, boyutu ya da aralığı olan metin de; stilli metin stilinin belirlediği şeyi korur. Değişen her belge önce bir anlık görüntü alır.

**Bir stil oluşturmak**:

1. Bir metni stilin görünmesini istediğiniz gibi biçimlendirin ve içine tıklayın ya da bir kısmını seçin.
2. **Biçim > Stil > Seçimden Yeni Stil…** seçin (ya da Stiller Panelinin artı düğmesi).
3. Bir ad girin ve neyin kaydedileceğini seçin: tüm biçimlendirme (karakter biçimlendirmeli bir paragraf stili), yalnızca paragraf biçimlendirmesi ya da karakter öznitelikleri (bir karakter stili).
4. Yazı tipinin ve yazı tipi boyutunun stilin parçası olup olmayacağına karar verin. Onlar olmadan stil, çevresindeki metnin yazı tipini korur.
5. Paragraf stilleri için, sonda Enter'a bastığınızda sonraki paragrafın hangi stili alacağını seçin.

Bir paragraf stili yalnızca gövde metninden farklı olanı saklar. Geri kalan her şey projenin biçimlendirmesini izlemeye devam eder, böylece yeni bir varsayılan yazı tipi stilli metne de ulaşır.

**Bir stili değiştirmek**:

- **Biçim > Stil > Seçimden Stili Yeniden Tanımla…** yukarıdaki gibi imlecin bulunduğu metnin biçimlendirmesini alır. Bir paragraf stili paragraf stili, bir karakter stili karakter stili olarak kalır.
- **Biçim > Stil > Stili Düzenle** (ya da Stiller Panelinin kalem düğmesi) ad, **Yazı Tipi…**, **Paragraf…** ve sonraki paragrafın stiliyle bir pencere açar. Bir önizleme sonucu gösterir.

Ardından {{APP}} bütün projeyi dolaşır ve metni açık belgelerde ve diğerlerinin dosyalarında günceller. Değişikliğin dokunduğu şeyler (yazı tipi, boyut, kalın, italik, renk, harf aralığı, büyük harfler, hizalama, girintiler, aralıklar, satır aralığı, sekme durakları) stilin metninin **tamamına** ulaşır, o metnin kendi değeri olduğu yerlere de: elle ayarlanmış ya da bir içe aktarmayla gelmiş (içe aktarılan başlıklar genellikle her paragrafta yazı tipini, boyutu ve aralığı belirtir). Değişikliğin dokunmadığı biçimlendirme olduğu gibi kalır, örneğin yalnızca başlığın boyutu değiştiğinde bir başlıkta elle renklendirilmiş bir sözcük. Değişen her belge önce bir anlık görüntü alır ("Before the style ... changed"). Bu **Düzenle > Geri Al** ile geri alınamaz; bir anlık görüntüyü geri yükleyin, stili yeniden tanımlayın ya da yerleşik bir stil için Stiller Panelindeki **Reset to the Built-in Definition** seçeneğini kullanın.

**Yeniden adlandırma ve silme**: bir stili **Stili Düzenle** içinde yeniden adlandırın. Başlıklar, Ana Başlık, Alt Başlık, Blok Alıntı, Ortalanmış ve Resim Yazısı adlarını korur, çünkü Derle ve Markdown dosyaları onları adlarıyla tanır. Bir stili sildiğinizde (**Biçim > Stil > Stili Sil**), onu kullanan metin görünümünü elle ayarlanmış biçimlendirme olarak korur.

**Diğer projeler**: **Biçim > Stil > Stilleri İçe Aktar…** başka bir projenin stillerini okur. **Use These Styles in New Projects** açık projenin stillerini her yeni projenin başlangıç kümesi yapar; **Dosya > Seçenekler > Düzenleme** yerleşik stillere geri döner.

Stiller Panelinde ok düğmeleri stillerin sırasını tüm listelerde değiştirir.

### Markdown veya RTF: belgeler nasıl saklanır

Her proje belgelerini iki biçimden birinde saklar. Bir projeyi oluştururken ya da içe aktarırken seçersiniz ve daha sonra **Proje > Proje Ayarları > Biçimlendirme** altında değiştirebilirsiniz.

- **Markdown** (önerilen): şimdi ve elli yıl sonra da herhangi bir programın açabileceği düz metin dosyaları. Metnin görünümü varsayılan biçimlendirmeden gelir. Elle ayarladığınız yazı tipleri ve paragraf biçimlendirmesi, yalnızca kullandığınız yerlerde küçük işaretler olarak metne yazılır (bölüm 18); geri kalan her şey düz kalır.
- **RTF**: bir kelime işlemci gibi zengin metin dosyaları. Her paragraf eksiksiz biçimlendirmesini taşır, bu nedenle varsayılan biçimlendirmenin değişmesi mevcut metne ulaşmaz.

Değiştirmek her belgeyi dönüştürür. RTF'den Markdown'a, yazı tipleri, renkler, hizalama, sol ve sağ girintiler ve sekme durakları korunur. Başka programlardan gelen metnin satır aralığı, ilk satır girintisi ve paragraf aralığı sonradan projeyi izler. Önce bir yedek alın (**Dosya > Yedekle > Şimdi Yedekle**).

Başka programlardan bir Markdown projesine yapıştırılan metin, çevresindeki metnin görünümünü alır (kalın, italik ve bağlantılar kalır). {{APP}}'in içinde kopyalanan metin yazı tiplerini korur. **Düzenle > Biçimlendirmeyle Yapıştır** (**Ctrl+Alt+Shift+V**) bir kelime işlemciden ya da bir web sayfasından gelen metnin görünümünü de korur: yazı tipleri, boyutlar, renkler ve paragraflarının hizalaması, girintileri ve aralıkları (yapıştırılan ilk paragraf, paragraf biçimlendirmesini yalnızca boş bir paragrafa getirir). **Düzenle > Stile Uyarak Yapıştır** (**Ctrl+Shift+V**) bunun tersini yapar ve düz metin yapıştırır. Metnin sağ tık menüsü üçünü de alt alta sunar: **Yapıştır**, **Stile Uyarak Yapıştır** ve **Biçimlendirmeyle Yapıştır**.

Bir projenin dışındaki tek bir dosyayı dönüştürmek için **Dosya > Markdown ⇄ RTF Dosyasını Dönüştür…** komutunu kullanın.

### Yorumlar ve dipnotlar

- **Yorum** (**Shift+F2**): bir metin seçin ve bir yorum ekleyin. Metin vurgulanır ve yorum denetçide görünür.
- **Dipnot** (**Ctrl+Shift+F2**): dipnotun ait olduğu sözcüğü seçin; dipnotu denetçide yazın.
- **Satır içi açıklama** (**Ctrl+Shift+A**): doğrudan metne yazılan, bir balonun içinde renkli gösterilen bir not. Açıklamalar derlediğinizde kaldırılır (onları yorum olarak dışa aktarmayı seçmediyseniz).
- **Satır içi dipnot** (**Ctrl+Alt+F**): doğrudan metne yazılan bir dipnot.

Bir satır içi açıklamanın rengini değiştirmek için onu seçin (**Düzenle > Seç > Açıklamayı veya Notu Seç** imlecin bulunduğu notun tamamını seçer) ve bir metin rengi seçin; açıklama o rengi korur. **Biçim > Yazı Tipi > Otomatik Metin Rengi** ona olağan kırmızıyı yeniden verir.

Atlamak için denetçide bir yoruma tıklayın. Rengini değiştirmek, onu başka bir tür nota dönüştürmek ya da silmek için sağ tıklayın.

Yorumlu metne tıklamak denetçiyi o yorumla açar. **Alt+tıklama** ise yorumu ya da dipnotu bunun yerine farenin yanında küçük bir pencerede açar; orada denetçi olmadan okuyabilir ve değiştirebilirsiniz; kapatmak için başka bir yere tıklayın ya da **Esc** tuşuna basın.

### Resimler ve bağlantılar

- **Ekle > Dosyadan Resim…** metne bir resim koyar; bir resmi metne sürükleyebilir ya da yapıştırabilirsiniz de. Ne kadar büyük görüneceğini değiştirmek için resme sağ tıklayın ve **Image Size…** seçin.
- **Ekle > Belgeye Bağlantı…** (**Ctrl+L**) seçili metni projedeki başka bir belgeye bağlar. Bağlantıya tıklamak o belgeyi açar (diğer düzenleyicide, aynı düzenleyicide, referans bölmesinde ya da bir Hızlı Başvuru panelinde; bkz. **Dosya > Seçenekler > Davranışlar**).
- **Ekle > Web Bağlantısı Ekle…** bir web sayfasına bağlar.
- Web ve posta bağlantıları (*http*, *https*, *mailto*) hemen açılır. Bir metindeki, bir zihin haritasındaki, bir yer imindeki ya da içe aktarılmış bir web sayfasındaki bir bağlantının diğer tüm hedefleri (bir dosya, bir klasör, bir ağ adresi, başka bir programın bağlantı türü) önce tam olarak gösterilir ve yalnızca **Aç** düğmesine tıkladığınızda açılır; bu tür bağlantılar başka birinin bir projesiyle, web sayfasıyla ya da Word dosyasıyla gelebilir. Bilgisayarınızdaki bir dosya ya da klasör için soru türünü (örneğin "plain text document (.txt)"), tam adını ve klasörünü belirtir; uzun bir ad sarılır, böylece sonu okunabilir ve orada olmayan bir dosyanın eksik olduğu söylenir. Programlara, betiklere, kısayollara ve yükleyicilere bağlantılar asla açılmaz; onlara güveniyorsanız dosya yöneticinizden kendiniz açın.

### Tablolar

**Ekle > Tablo…** satır ve sütun sayısını ve tasarımı sorar ve tabloyu imlecin olduğu yere koyar. Bir tablonun içinde **Tab** sonraki hücreye gider (son hücrede bir satır ekler) ve **Shift+Tab** geri gider. Tablonun içindeki sağ tık menüsünde de bulunan **Biçim > Tablo**, geri kalanını içerir:

- **Yukarıya / Aşağıya Satır Ekle**, **Sola / Sağa Sütun Ekle**.
- **Satırları Sil** ve **Sütunları Sil** (seçili hücrelerin ya da imlecin bulunduğu hücrenin), **Tabloyu Sil**.
- **Hücreleri Birleştir** seçili hücreleri tek hücrede birleştirir; **Hücreyi Böl** birleştirilmiş bir hücreyi yeniden böler.
- **Hücre Rengi…** seçili hücrelere kendi arka planını verir; **Tasarımın Hücre Rengi** onu yeniden kaldırır.
- **Tablo Özellikleri…** tasarımı değiştirir.

**Tasarım.** Beş hazır tasarım vardır: **Grid**, **Grid with a header row**, **Striped rows**, **Lines between the rows** ve **No lines**. Birini seçin ve istediğinizi değiştirin; tasarım o zaman **My own** olur:

- **Çizgiler**: genişlik (0: çizgi yok), renk ve **Only between the rows** (yanlarda çizgi yok).
- **Hücreler**: hücrenin içindeki boşluk ve hücreler arasındaki boşluk (0: komşu hücreler tek bir çizgiyi paylaşır).
- **Başlık satırları**: üstteki kaç satırın başlık olduğu, renkleriyle birlikte. Yazdırma ve derleme, uzun bir tablonun devam ettiği her sayfanın üstünde onları yineler. Bir renk seçerseniz **Every other row** kendi rengini alır.
- **Metindeki yeri**: genişlik (metin genişliğinin bir payı ya da içeriği kadar geniş), sütun genişlikleri (hepsi aynı ya da "30, 70" gibi paylar), hizalama ve nerede durduğu: **On its own lines** ya da metnin yanından aktığı **Left** veya **Right**.

Sonradan eklediğiniz satırlar ve sütunlar tasarımı korur (şeritler devam eder). Tablo hücrelerindeki metin, gövde metni iki yana yaslı olduğunda da yaslanmaz (dar bir hücre sözcüklerini yayardı); bir hücredeki bir paragrafa verdiğiniz hizalama kalır. Tablolar her iki saklama biçiminde ve Scrivener biçiminde saklanır; başka programlardan gelen RTF dosyalarındaki tablolar (örneğin RTF olarak kaydedilmiş bir Word dosyası) da tablo olur.

### Metnin yanındaki resimler, resim yazıları ve açıklamalar

Bir resme sağ tıklayın ve **Resim…** seçin (ya da imleci yanına koyun ve **Biçim > Resim…** seçin):

- **Ekrandaki genişlik**: yükseklik onu izler; **Original Size** resmin kendi boyutuna döner.
- **Konum**: **In the line, like a letter** (eskisi gibi), metnin çevresinden aktığı **Left** ya da **Right** veya **Centered on its own lines**.
- **Resim yazısı**: resmin altında gösterilir ve onunla birlikte derlenir. Satır içindeki bir resmin yazısı olmaz; önce ona bir konum verin.
- **Açıklama**: resmin ne gösterdiği, bir iki cümleyle. Metinde gösterilmez. E-kitaplar, Word dosyaları ve Markdown onu resimle birlikte saklar ("alt metin" olarak) ve ekran okuyucular onu resmi göremeyen insanlara okur.

Resim yazısıyla birlikte resim tek parça olarak taşınır; resim yazısını resmin altına diğer her metin gibi yazın (stili **Resim Yazısı**'dır).

### Bul ve değiştir

- **Ctrl+F** (**Düzenle > Bul > Bul**) metnin üstünde bul çubuğunu açar. Kaç eşleşme olduğunu gösterir ve onları vurgular; **F3** ve **Shift+F3** sonraki ve önceki eşleşmeye gider. **Match case** ve **Whole words** aramayı daraltır.
- **Ctrl+H** değiştirme satırını ekler: **Replace** seçili eşleşmeyi değiştirir ve sonrakine gider, **Replace All** gördüğünüz metindeki her eşleşmeyi değiştirir (**Geri Al** için tek adım). Yeni metin, yerini aldığı metnin biçimlendirmesini devralır ve yorumlar bağlı kalır.
- **Ctrl+E** (**Seçimi Arama İçin Kullan**) seçili metni bul çubuğuna koyar.
- Birleşik metinde bul ve değiştir, gösterilen tüm belgeler boyunca çalışır.
- Klavye bir **referans bölmesindeyken**, Ctrl+F referans bölmesinin belgesinde arar.
- **Ctrl+Shift+F** bütün projede arar, **Düzenle > Bul > Projede Değiştir** bütün projede değiştirir ve **Düzenle > Bul > Biçimlendirmeye Göre Bul** vurguları, yorumları, renkleri ve stilleri arar (hepsi bölüm 9'da).
- Mantar panoda ve anahatta Ctrl+F bunun yerine **süzgeç çubuğunu** açar (bölüm 5).

### Yazım denetimi

Yanlış yazılmış sözcükler kırmızı dalgalı bir altçizgi alır; önerileri görmek ya da sözcüğü sözlüğünüze eklemek için altı çizili sözcüğün herhangi bir yerine sağ tıklayın (kesme işareti olan sözcüklerde de, örneğin "Kayel's" ya da "D'Nersun": menü her zaman altı çizili sözcüğün tamamını sunar). Yanlış yazılmış bir sözcükte menü kısadır, böylece tam farenin yanında açılır: öneriler, **Add "..." to Dictionary**, **Ignore "..."** ve menünün başka yerde tuttuğu her şeyle **More Commands** (Kes, Kopyala, Yapıştır, Stil, yorumlar, Dış Görünüm ...). Programın kendi adı ve terimleri ({{APP}}, anahat, alt belge, referans bölmesi) her zaman doğru sayılır. Dili **Dosya > Seçenekler > Düzeltmeler** altında seçin (Yazım Denetimi sayfası); eklenti olarak kurduğunuz sözlükler (programın her dili için bir tane, her biri kendi lisansıyla) bu listededir; orada ayrıca altçizgiyi (dalgalar, noktalar ya da düz çizgi), sayılı sözcüklerin ("Route66") denetlenip denetlenmeyeceğini seçersiniz ve **View Personal Word List...** eklediğiniz sözcükleri gösterir ve yanlışlıkla eklediklerinizi kaldırır. Sözlükler sisteminizden ve eklentilerden gelir. Ubuntu ve Kubuntu'da İngilizce kuruludur; diğer diller için Discover'da uygun *hunspell* paketini kurun (örneğin Almanca için "hunspell-de-de").

### Özel eklemeler

**Ekle** menüsü ayrıca geçerli tarih ve saati, özel karakterleri (uzun çizgi, orta çizgi, üç nokta, bölünmeyen boşluk, paragraf işareti, madde işareti), **otomatik numaraları** ve Derle'nin değiştirdiği **yer tutucuları** sunar (bölüm 13).

### Yazarken düzeltmeler

**Dosya > Seçenekler > Düzeltmeler**, yazarken nelerin düzeltileceğini ayarlar. Bir sözcük bitirdiğinizde, yani ardından bir boşluk, bir Return ya da bir noktalama işareti yazdığınızda düzeltilir; hemen ardından **Ctrl+Z** düzeltmeyi tek başına geri alır.

- **Fix capitalization of sentences**: bir cümlenin ilk sözcüğü büyük harf alır ("Dr." ya da "e.g." gibi kısaltmalardan sonra değil). **Capitalize "i"**: "i" ve "i'm" "I" ve "I'm" olur. İkisi de başlangıçta kapalıdır.
- Seçtiğiniz stilde **akıllı tırnaklar** (“English”, „German“, »German«, «French», ”Swedish”); bir sözcüğün içindeki kesme işareti her zaman ’ olur. Kapanış tek işaretinin kesme işareti olmadığı yerlerde (Almanca ‚…‘, Fransızca ‹…›), bir sözcüğün hemen ardından yazılan bir ' paragrafta açık olan bir tek işareti kapatır; sonra harf yazmaya devam ederseniz ("geht's") yeniden kesme işareti olur. İki kısa çizgi bir uzun çizgi (—) olur; üç nokta tek bir üç nokta karakteri olabilir (başlangıçta kapalı). Senaryo modunda bunlar kapatılabilir.
- **Projede zaten bulunan tırnak işaretleri** (0.61.1'den beri): **Smart quotes** altında **Convert to Typographic Quotes…** projenin düz " ve ' işaretlerini **Use smart quotes** yanında seçilen stilin işaretlerine dönüştürür ve **Convert to Straight Quotes…** her tipografik tırnak işaretini ve kesme işaretini yeniden " ve ' yapar. Projede Değiştir gibi bir pencere açılır (bölüm 9): yön ve stil bir kez daha, nerede dönüştürüleceği (başlıklar, metin, notlar, özetler, özel üst veri, yorumlar ve dipnotlar), kapsam (bütün proje ya da binder'da seçili belgeler, Çöp Kutusu yalnızca işaretlediğinizde) ve değişmeden önce her metnin bir anlık görüntüsü (başlangıçta işaretli). **Count** kaç işaretin değişeceğini gösterir; **Convert** bir kez daha sorar ve yapar. Biçimlendirme kalır (kalın bir tırnak işareti kalın kalır). Bir işaret hiçbir şeyden, bir boşluktan, bir parantezden, bir çizgiden ya da bir açılış işaretinden sonra açılır ve aksi halde kapanır; bir sözcüğün içindeki ya da bir sayıdan önceki bir kesme işareti ('90s) ’ olur. İki yanında boşluk olan düz bir işaret olduğu gibi kalır ve bir sözcüğün başındaki bir kesme işareti ('tis) bir açılış işareti olur: sonradan onlara bakın. Bul ve Değiştir bunu yapamaz, çünkü bir işaretin komşularına göre açılış ya da kapanış işareti olması gerekir.
- **Veri algılama**: yazılan bir web adresi bir bağlantı olur; **[[Title]]** o başlıklı belgeye bir bağlantı olur ve yoksa metnin yanında o adda yeni bir boş belge yapılır (Scrivener'ın "belge bağlantıları").
- **Ek değiştirmeler**: yazdığınız ve olacağı şeylerin listesi, örneğin "(c)" ve ©, "->" ve →. **Edit Substitutions...** listeyi değiştirir; **Standard Substitutions** programın geldiği listeyi geri getirir.
- **Otomatik tamamlama**: senaryo modunda, yazarken sunulan karakter adları ve sahne başlıkları listesi.

### Yakınlaştırma, daktilo kaydırma ve sabit genişlik

- **Ctrl+fare tekerleği**, farenin altındaki alanı, nerede olursa olsun yakınlaştırır: düzenleyiciler, binder, denetçi, anahat, mantar pano (kartlar ve metinleri birlikte), bir zihin haritası, yazım modu ve kendi pencereleri (Hızlı Başvuru, yazım araçları); klavye başka bir yerde olabilir. Her alan, onu yeniden değiştirene kadar yakınlaştırmasını korur.
- **Ctrl++ / Ctrl+-** ya da alt bilgideki yakınlaştırma kutusu, yazı tipi boyutunu değiştirmeden ekrandaki metni büyütür. **Görünüm > Yakınlaştırmayı Sıfırla** (**Ctrl+0**) varsayılan metin yakınlaştırmasına döner.
- **Dosya > Seçenekler > Düzenleme**, her alanın başladığı yakınlaştırmayı ("Default text zoom", pencerenin her alanı ve kendi yakınlaştırması olan her bölüm için: özet, notlar, yer imi listeleri, yer imli metin, anlık görüntü listesi ve metni, denetçide ve Nesneler penceresinde nesne listesi ve sayfası, yeni zihin haritaları) ve düzenleyicilerin en son sahip oldukları yakınlaştırmayla yeniden açılıp açılmayacağını ("Reload the main editors' last selected zoom") ayarlar.
- **Kaldığınız yer.** {{APP}} projeyle birlikte her belge için imlecin yerini ve ekrandaki metin bölümünü hatırlar: yeniden gösterildiğinde (binder'dan, bir bağlantıdan, Geri ve İleri ile, referans bölmesinde, bir Hızlı Başvuru panelinde ya da projeyi yeniden açtıktan sonra), bir belge bıraktığınız yerde açılır. Birleşik metindeki bir klasör, hangi belgesinde imleç olduğunu hatırlar. Belirli bir yeri gösteren bir arama sonucu ya da bir bağlantı bunun yerine oraya gider. Resimler yakınlaştırmalarını, PDF dosyaları yakınlaştırmalarını ve sayfalarını korur.
- **Daktilo kaydırma** (**Görünüm > Metin Düzenleme**) yazdığınız satırı ekranın ortasında tutar.
- **Sabit genişlikli düzenleyici**, pencere ne kadar geniş olursa olsun, metni rahat genişlikte (değiştirmediyseniz %100'de 500 px) ortalanmış bir sütun olarak gösterir. Onu **Dosya > Seçenekler > Dış Görünüm > Ana Düzenleyici** altında ("Use fixed width editor", "Text column width") ya da kâğıda sağ tıklayarak (**Dış Görünüm > Text Column Width**) kapatın ya da genişliği değiştirin. Sütunun kâğıdı standart olarak #d1d495, yanındaki alan #999999'dur (Ana Düzenleyici renkleri arasında "Paper" ve "Beside the text column"); başka bir tema seçtiğinizde, onları değiştirene ya da sıfırlayana kadar aynı kalırlar.
- **Metnin yanındaki kenarlık.** Metnin solunda ve sağında kâğıdın kendi renginde bir kenarlığı vardır, böylece metin kâğıdın kenarına değmez: yeni projeler için %100'de 10 px ve yakınlaştırmayla büyür. Metin sütununun dışında yer alır, böylece satırlar eskisi gibi kırılır. Onu **Dosya > Seçenekler > Dış Görünüm > Ana Düzenleyici** altında ("Border beside the text", 0 ila 100 px) ya da kâğıda sağ tıklayarak (**Dış Görünüm > Border Beside the Text**) değiştirin. 0.40.2'den önce yapılan projeler, onlara bir tane verene kadar, kenarlıksız eski görünümlerini korur. Kenarlığa tıklamak imleci yanındaki satırın başına ya da sonuna koyar.
- **Tıklama ve sağ tıklama** görünümü asla oynatmaz: imleç tıkladığınız yere gider ve metin ekranda olduğu yerde kalır (0.40.2'den önce, üst ya da alt kenara yakın bir tıklama ya da sağ tıklama ya da metnin yanında bir sağ tıklama metnin sıçramasına yol açabilirdi).

### Sayfa görünümü

**Görünüm > Metin Düzenleme > Sayfa Görünümü** (**Görünüm** araç çubuğu düğmesinin menüsünde de) metninizi sonsuz tek bir sütun yerine kâğıt sayfalar üzerinde gösterir. Alt bilgi o zaman imlecin sayfasını ve sayfa sayısını gösterir, örneğin "Page 3 of 12". Sütuna dönmek için komutu yeniden seçin. Sayfaların çevresindeki masanın rengi **Dosya > Seçenekler > Dış Görünüm > Ana Düzenleyici** altındadır (ya da sayfalar gösterilirken sağ tık > **Dış Görünüm**).

Sayfalar metninize bakmanın bir yoludur, daha fazlası değil. Derlenmiş ya da yazdırılmış bir müsveddenin nasıl görüneceğini göstermezler: parçalar halinde yazarsınız ve başlıklar, yazı tipleri ve sayfa boyutu derlediğinizde belirlenir (bölüm 13). Sayfalarla ilgili hiçbir şey belgelerinizde saklanmaz.

- **Görünüm > Metin Düzenleme > Sayfa Boyutu ve Kenar Boşlukları…** kâğıdı (A4, A5, US Letter ve diğerleri ya da kendi boyutunuz), dikey ya da yatay yönü ve dört kenar boşluğunu ayarlar. Ayar projeye aittir; onu standardınız yapmak için **Use this page for new projects as well** kutusunu işaretleyin.
- Yakınlaştırma sayfaları, kenar boşluklarını ve harfleri birlikte büyütür. Düzenleyiciden daha geniş bir sayfa yana kaydırılabilir. Sütun görünümünde olduğu gibi girintiler ve sekme durakları gerçek uzunluklarını korur, bu nedenle satır sonları yakınlaştırma düzeyleri arasında biraz farklı olabilir.
- Birleşik metinde her belge kendi sayfasında başlar ve son sayfası metnin hemen altında biter; böylece kısa belgeler ekranı boş kâğıtla doldurmaz.
- Sayfa görünümü, ana pencerede ve hızlı başvuru pencerelerinde metinlerinizi gösteren her şeye uygulanır: her iki düzenleyici ve referans bölmeleri. Notlar düz görünümlerini korur ve yazım modunun kendi kâğıdı vardır.
- Sayfaların çevresindeki renk, **Dosya > Seçenekler > Dış Görünüm > Ana Düzenleyici** altındaki **Around the pages (page view)** rengidir.

### Senaryo yazımı

Senaryolar, sahne oyunları ve çizgi roman metinleri **senaryo modunda** yazılır. Bir senaryo *öğelerden* oluşur: sahne başlığı, aksiyon, karakter, parantez içi yönerge, diyalog, geçiş ve benzerleri. Her öğenin sayfada kendi yeri vardır ve senaryo modunda **Tab** ve **Enter** tuşları sizi bir öğeden diğerine götürür; böylece hiçbir zaman elle girinti ayarlamanız gerekmez.

**Açmak.** Bir belgenin içine tıklayın ve **Biçim > Senaryo Yazımı > Senaryo Modu** (**Ctrl+8**) seçin. Menü girişi projenin senaryo biçimini adlandırır, örneğin *Script Mode - Screenplay*. Senaryo modu belgeye aittir: senaryolar ve sıradan metinler tek bir projede yan yana yaşar ve senaryoların binder'da **sarı bir simgesi** vardır. Kapatmak için komutu yeniden seçin; metin görünümünü korur. Senaryoların arasında oluşturulan yeni bir belge (metinlerinin hepsi senaryo olan bir klasörde) de bir senaryodur.

**Alt bilgi.** Bir senaryoda düzenleyicinin alt bilgisi, sözcük sayısı yerine, **Tab** ve **Enter** tuşlarının imlecin olduğu yerde ne yapacağını ve sağda geçerli paragrafın öğesini içeren bir açılır menü gösterir. O menüden başka bir öğe seçin ve paragraf yeniden biçimlendirilir. Senaryonun bir öğesi olmayan metin **General Text** olarak gösterilir.

**Tab ve Enter.**

- Bir paragrafın sonunda **Enter**, genellikle ardından gelen öğeyi başlatır: sahne başlığından sonra aksiyon, karakter adından sonra diyalog ve benzerleri.
- Boş bir satırda **Enter**, imlecin olduğu yerde öğelerin listesini açar; listede altı çizili harfe basın (sahne başlığı için S, karakter için C ...). **Biçim > Senaryo Yazımı > Öğe Listesini Göster** (**Ctrl+\\**) aynı listeyi istediğiniz zaman açar.
- Boş bir satırda **Tab** satırı başka bir öğeye değiştirir; sonraki için yeniden basın, geri dönmek için **Shift+Tab**.
- Bir paragrafın sonunda **Tab**, öğeye uygun olanı yapar: bir karakter adından ya da bir diyalog satırından sonra bir parantez içi yönerge başlatır ve bir sahne başlığında günün saati için hazır bir " - " yazar.
- Bir paragrafın ortasında **Enter** onu böler; her iki yarı da öğeyi korur. **Shift+Enter** paragrafın içinde bir satır sonudur.
- Büyük harfle yazılan öğeler (sahne başlıkları, karakter adları, geçişler) yazdığınızı büyük harfe çevirir. Parantezler ve "SFX: " gibi önekler sizin için yazılır.

**Otomatik tamamlama.** Yazarken, küçük bir liste öğeye uyanı sunar: sahne başlığının başında INT. ve EXT., onun " - " işaretinden sonra günün saatleri, belgede zaten kullanılmış yerler ve karakter adları ve CUT TO: gibi hazır geçişler. **Enter** ya da **Tab** işaretli girişi alır, ok tuşları başka birini seçer, **Esc** listeyi kapatır.

**Öğeleri değiştirmek.** **Biçim > Senaryo Yazımı > Öğeyi Şuna Değiştir**, imlecin bulunduğu paragrafı ya da seçili tüm paragrafları değiştirir. Orada sıradan bir belgede bir öğe seçmek belgeyi bir senaryo yapar. Her öğe bir komuttur, böylece sevdiklerinize **Dosya > Seçenekler > Klavye** altında bir kısayol verebilirsiniz.

**Senaryo biçimleri.** Bir proje, tüm senaryoları için tek bir senaryo biçimi kullanır. **Biçim > Senaryo Yazımı > Senaryo Biçimi**, *Screenplay*, *Stage Play* ve *Comic Script*'i, eklentilerin biçimlerini ve kendi biçimlerinizi listeler. Başka bir biçim seçtiğinizde, projenin senaryoları yeni düzeni alır: her öğe, yeni biçimde aynı türden öğe olur (örneğin aksiyon bir sahne yönergesi olur). Karşılığı olmayan bir öğe, görünümünü koruyan genel metin olur; değişiklikten önceki soru bu öğeleri adlandırır.

**Senaryo Ayarları.** **Biçim > Senaryo Yazımı > Senaryo Ayarları…** projenin biçimini değiştirir ya da yeni bir tane oluşturur:

- biçimin adı, yazı tipi (varsayılan olarak sisteminizin daktilo yazı tipi: Courier Prime, Courier New ya da benzeri bir yazı tipi) ve boyutu ve uzunlukların inç mi yoksa santimetre mi olduğu;
- öğelerin listesi: ekleyin, kaldırın ve yeniden sıralayın (sıra, öğeler listesinin ve boş bir satırda **Tab** tuşunun sırasıdır);
- her öğe için: ad, tür (diğer programlara göre ne olduğu: sahne başlığı, aksiyon, karakter ...) ve listedeki harfi; kalın, italik, altı çizili, büyük harfler ve renk; hizalama, iki kenar boşluğundan girintiler, öncesinde boş satırlar ve satır aralığı; **Enter** ve **Tab** tuşlarının ne yaptığı; paragrafın öncesinde ve sonrasında yazılan bir metin; ve otomatik tamamlama için listeler;
- tüm öğelerin bir önizlemesi.

Yeniden adlandırılan bir öğe, projenin her senaryosunda paragraflarını korur. **Yönet** düğmesi başka bir biçimden başlar, bir biçim dosyasını (*.qbscript*) yükler ya da kaydeder ve biçimi başka projelerde kullanmak üzere kaydeder.

**Bilmekte yarar var.**

- Tuşlar ve listeler birleşik metinde, ikinci düzenleyicide, referans bölmelerinde, Hızlı Başvuru panellerinde ve yazım modunda aynı şekilde çalışır. İpuçlu alt bilgi yalnızca ana pencerenin düzenleyicilerinde vardır; başka yerlerde **Ctrl+\\** öğeler listesini gösterir.
- Sayfa görünümü (yukarıda) bir senaryoyu sayfalar üzerinde gösterir; 12 puntoluk olağan daktilo yazı tipiyle bir senaryonun bir sayfası yaklaşık bir dakikalık filmdir, ancak ekrandaki sayfalar derlenen dosyanın sayfaları değildir.
- Öğelerin yazı tipi ve girintileri senaryo biçimine aittir, belgeye değil. Senaryolarınızın dosyaları yalnızca öğelerin adlarını içerir (bölüm 18).
- Senaryoları diğer senaryo yazım programlarının biçimlerinde almak ve vermek: bölüm 14. Bütün bir senaryoyu derlemek: bölüm 13.
- Senaryolar 0.10 ya da daha sonraki sürümü gerektirir. Senaryo içeren bir proje bunu proje dosyasında söyler ve bu sürümden itibaren {{APP}}, çalışan sürümden daha yeni bir sürüm gerektiren bir projeyi açmadan önce sorar. 0.9 ve daha eski sürümler sormaz: senaryoları sıradan metin olarak gösterir ve orada baktığınız her senaryonun öğelerini kaybeder.

---

## 5. Projenize bakmanın üç yolu

Görünümü üç araç çubuğu düğmesiyle ya da **Ctrl+1**, **Ctrl+2** ve **Ctrl+3** ile seçin.

### Birleşik Metin (Ctrl+1): birçok belge tek bir metin olarak

Binder'da birkaç belge (Ctrl+tıklama ya da Shift+tıklama) ya da bir klasör seçin ve **Ctrl+1** tuşuna basın. Tüm metinler ince kesikli bir çizgiyle ayrılarak art arda görünür ve hepsine tek bir belgeymiş gibi yazabilirsiniz. Değişiklikler kendiliğinden doğru belgelere gider. Aralarında belge başlıklarını **Görünüm > Metin Düzenleme > Birleşik Metinde Başlıkları Göster** ile gösterin.

### Mantar Pano (Ctrl+2): dizin kartları

Her belge, başlığını ve özetini gösteren bir dizin kartıdır; özeti olmayan bir belge bunun yerine metninin ilk satırlarını gösterir (soluk, italik). Bir resmin kartı resmi gösterir, özeti bir resim olan bir belgenin kartı da öyle (bölüm 7). Belgeleri yeniden sıralamak için kartları sürükleyin. Başlığını ya da özetini düzenlemek için bir karta çift tıklayın. Seçili kartları metin olarak açmak için **Enter** tuşuna basın.

Mantar pano alt bilgisinde:

- **Grid** ya da **freeform**: serbest biçim modunda kartları gerçek bir mantar panoda olduğu gibi her yere yerleştirebilirsiniz. **Commit Order** o zaman binder'daki belgeleri kartlara uyacak şekilde soldan sağa, yukarıdan aşağıya yeniden sıralar. Bir klasör için seçtiğiniz düzen o klasörle birlikte hatırlanır (0.90.0'dan beri): onu bir sonraki gösterişinizde, mantar panosu aynı şekilde düzenlenir. Hiçbir zaman bir düzen seçmediğiniz bir klasör, düzenleyicinin gösterdiği düzeni korur.
- **Label threads**: kartları etikete göre satırlara dizer.
- Kartların **Card size** boyutu, kaydırıcıyla, ok tuşlarıyla ya da üzerindeki fare tekerleğiyle; hemen saklanır (mantar pano üzerinde **Ctrl+fare tekerleği**, diğer alanlar gibi kartları ve metinlerini birlikte yakınlaştırır, bkz. bölüm 4).

Mantar panonun boş bir yerine çift tıklamak yeni bir kart yapar (**Dosya > Seçenekler > Davranışlar > Çift Tıklama**). Bir karttaki özeti düzenlerken **Return** düzenlemeyi bitirir ve **Shift+Return** yeni bir satır yapar (**Dosya > Seçenekler > Davranışlar > Return Tuşu**; orada Return, seçilenden sonra yeni bir kart ya da anahat satırı yapacak şekilde de ayarlanabilir).

Durum damgaları, serbest biçim modu ve etikete göre düzen de **Görünüm > Mantar Pano Seçenekleri** altındadır; kartlar ve binder için etiket tonları **Görünüm > Etiket Rengini Şurada Kullan** altındadır. Anahatın sütunları da **Görünüm > Anahat Seçenekleri** altında listelenir.

### Anahat (Ctrl+3): bir tablo

Belgeleri başlık, özet, etiket, durum, sözcük sayıları, hedefler, tarihler ve özel üst verilerinizle bir tablo olarak gösterir. Sütunları seçmek için alt bilgideki sütun düğmesine tıklayın. Sıralamak için bir sütun başlığına tıklayın (yalnızca görüntüleme için; binder sırası değişmez). Belgeleri yeniden sıralamak için satırları sürükleyin.

### Mantar panoyu ve anahatı süzmek

Mantar pano ya da anahat klavyeye sahipken, **Ctrl+F** (**Düzenle > Bul > Bul**) düzenleyicinin üstünde bir **süzgeç çubuğu** açar. Yazdıkça yalnızca eşleşen kartlar ya da satırlar kalır; çubuk kaç tane olduğunu söyler ("2 of 9"). Anahatta, eşleşen belgelerin grupları kalır, böylece nereye ait olduklarını görürsünüz.

- Seçenekler için alanın solundaki **üçgene** tıklayın: nereye bakılacağı (**Başlıklar**, **Özetler**, **Anahtar Sözcükler**, **Metin**), bir **etiket**, bir **durum**, bir **anahtar sözcük**, belgenin **derlemeye dahil** olup olmadığı ve **Show what does not match**. Birlikte birkaç seçim listeyi daha da daraltır.
- Süzgeç, başka klasörler açarken de kalır, böylece projede örneğin yalnızca bir karakterin sahnelerine bakarak yürüyebilirsiniz.
- **Done** süzgeci kaldırır ve her şeyi yeniden gösterir.

Kartları sürüklemek yine belgeleri yeniden sıralar: gördüğünüz iki kartın arasına bırakılan bir kart, binder'da onların yanına iner. Serbest biçimli mantar panonun **Commit Order** komutu, bir süzgeç kartları gizlerken kapalıdır.

---

## 6. İki düzenleyici, yer imleri ve hızlı başvuru

### Bölünmüş düzenleyici

İkinci bir düzenleyici göstermek için düzenleyici başlığının sağındaki bölme düğmesine tıklayın. Alt+tıklama öbür yönde böler. **Görünüm > Düzenleyiciyi Böl** aynı seçenekleri sunar. **F6** düzenleyiciler arasında atlar. Binder'da tıklamak belgeyi en son kullandığınız düzenleyicide gösterir; **Gezin > Binder Seçimi Şunu Etkiler** bunu soldaki ya da üstteki düzenleyiciye, sağdaki ya da alttaki düzenleyiciye ya da diğer düzenleyiciye, hangisinde yazıyorsanız ona değiştirir.

Bir mantar panoda ya da anahatta, alt bilgi düğmesi **Selection affects the other editor** (**Görünüm > Mantar Pano Seçenekleri** ve **Anahat Seçenekleri** altında da) diğer düzenleyicinin tıkladığınız kartı göstermesini sağlar. Bu, bölümlerin genel görünümünü korurken sahnelere göz atmanın hızlı bir yoludur.

**Gezin > Düzenleyici > Yerine Kilitle**, binder'da başka bir yere tıklasanız bile bir düzenleyiciyi belgesinde tutar.

### Referans bölmeleri

Her düzenleyicinin bir **referans bölmesi** vardır: düzenleyicinin içinde, notlar ya da bir karakter sayfası gibi, başvuru için başka bir belgeyi tutan ikinci, sade bir görünüm. İki düzenleyici ve referans bölmeleriyle aynı anda dört belge görebilirsiniz.

- **Açmak için**, **Alt** tuşunu basılı tutun ve bir belgeyi binder'dan, bir mantar panodan ya da bir anahattan düzenleyicinin başlık çubuğuna sürükleyin. Alt olmadan bırakma belgeyi düzenleyicinin kendisine yükler. **Gezin > Aç > Referans Bölmesinde** ve bir belgenin sağ tık menüsü, seçili belge için aynısını yapar.
- Referans bölmesinin başlığı ve alt bilgisi olmayan dar bir başlık çubuğu vardır. Bir belgenin (ya da bir resmin ya da PDF'in) metnini gösterir; mantar panosu, anahatı ya da birleşik metni yoktur. İçinde herhangi bir düzenleyicide olduğu gibi yazabilir ve biçimlendirebilirsiniz.
- **Taşımak için**, başlık çubuğuna sağ tıklayın ve sol, sağ, üst ya da alt seçin. İlk seferinde geniş bir düzenleyicinin yanında ve dar bir düzenleyicinin altında açılır; ondan sonra en son koyduğunuz yerde. Boyutlarını değiştirmek için düzenleyici ile referans bölmesi arasındaki çizgiyi sürükleyin.
- **Başka bir belge göstermek için**, onu referans bölmesinin başlık çubuğuna bırakın ya da Alt tuşuyla yeniden düzenleyicinin başlık çubuğuna sürükleyin.
- **Kapatmak için**, başlık çubuğunun sağındaki **X** düğmesine tıklayın ya da **Gezin > Düzenleyici > Referans Bölmesini Kapat** seçin. X'in solundaki düğme onu kapatır ve belgeyi bunun yerine bir hızlı başvuru penceresi olarak açar.

**Referans bölmesiyle gezinmek.** Bir düzenleyici bir mantar pano ya da anahat gösterirken ve referans bölmesi açıkken, alt bilgideki **Selection affects** düğmesinin üç konumu vardır. İlk tıklama, seçiminizin her zamanki gibi diğer düzenleyicide açılmasını sağlar. İkinci tıklama oku kutunun içini gösterecek şekilde çevirir: şimdi tıkladığınız kart ya da satır referans bölmesinde açılır. Üçüncü bir tıklama onu kapatır. Yararlı bir düzen: solda müsvedde düzenleyicisi, sağda altında referans bölmesi olan araştırmanızın anahatı olan düzenleyici ve **Gezin > Binder Seçimi Şunu Etkiler > Soldaki veya Üstteki Düzenleyici**. Binder o zaman müsveddeyi, anahat ise referans bölmesini yönetir.

Referans bölmesi, yeri ve belgesi projeyle birlikte hatırlanır.

### Düzenleyici başlığı

Her düzenleyicinin üstündeki başlık belgenin başlığını gösterir ve **geri** ve **ileri** düğmelerini (**Ctrl+[** ve **Ctrl+]**), belgenin bölümlerinin bir menüsünü ve önceki ve sonraki belge için düğmeleri (**Alt+Shift+Yukarı / Aşağı**) içerir.

- **Geçmiş**: her düzenleyici gösterdiği belgeleri hatırlar. Bir liste için geri ya da ileri düğmesini basılı tutun (ya da sağ tıklayın) ve doğrudan gitmek için bir belge seçin; **Gezin > Şuraya Geri** ve **Şuraya İleri** aynı listeleri gösterir. Geçmiş projeyle birlikte saklanır.
- Başlığın yanındaki **simge** belgeyi temsil eder. Bir menü için ona tıklayın (Yerine Kilitle, Binder'da Göster ...). Onu binder'daki bir belge gibi sürükleyin: diğer düzenleyicinin başlık çubuğuna, binder'a, bir koleksiyon sekmesine, araç çubuğunun Hızlı Başvuru ya da Yer İmleri düğmesine ya da bir bağlantı yapmak için metne.
- **Belgeler > Grupla** (**Ctrl+Shift+J**, binder'ın bağlam menüsünde de) seçili öğeleri ilkinin yerinde yeni bir klasöre taşır; başlığı yazmaya hazırdır. Klavye binder'dayken **Düzenle > Geri Al** bütün adımı geri alır.
- **Belgeler > Dönüştür > Sayfayı Nesneye Dönüştür** (binder'ın bağlam menüsünde de), seçili her sayfa belgesinden, örneğin bir karakter ya da yer sayfasından, bir Nesne yapar (Nesneler bölümü): başlık ad olur, metin **Other** içine gider, metindeki bir resim resim olur. Klasörün adı (Characters, Places) türü seçer; başka bir klasör için size sorulur. Binder öğesi kalır: onu seçmek Nesneyi denetçide gösterir ve Return onu Nesneler penceresinde açar. Belge metnini korur (bir Scrivener projesi normal bir belge görür); bağlantı projenin kendi Nesneler verilerinde saklanır. Klavye binder'dayken **Düzenle > Geri Al** onu geri alır.
- **Gezin > Git**, bütün binder'ı bir menü olarak listeler ve seçilen belgeyi etkin düzenleyiciye yükler. **Gezin > Kapsayan Grup**, belgenin ait olduğu klasörü ya da grubu, belge orada seçili olarak gösterir.
- İki düzenleyici arasındaki **ayırıcıya çift tıklamak** onları aynı boyuta getirir.

### Bir bakışta sürükle ve bırak

| Sürükleyin ... | şunun üzerine ... | ve ... |
|---|---|---|
| bir belge (binder, koleksiyon, mantar pano, anahat, başlık simgesi, yer imi) | bir düzenleyicinin başlık çubuğu | o düzenleyicide gösterilir; **Alt** ile: referans bölmesinde |
| | bir düzenleyicideki metin | belgeye bir bağlantı; seçili metinde o metin bağlantı olur |
| | metin, **Alt** basılıyken | belgenin metni eklenir |
| binder'dan bir resim | bir düzenleyicideki metin | resim eklenir |
| bir belge | araç çubuğunun **Hızlı Başvuru** düğmesi | her belge kendi panelinde açılır |
| | araç çubuğunun **Yer İmleri** düğmesi | proje yer imlerine eklenir |
| | bir koleksiyon sekmesi, bir yer imi listesi | eklenir |
| | bir Hızlı Başvuru panelinin başlık çubuğu | o panelde gösterilir |
| | dosya yöneticisi ya da masaüstü | oraya bir dosya olarak kaydedilir (bkz. "Belgeleri dışa aktarmak") |
| seçili metin | binder | o metinle yeni bir belge; metin olduğu yerde kalır |
| dosya yöneticisinden dosyalar | binder ya da bir mantar pano | içe aktarılırlar |
| bir tarayıcıdan bir web bağlantısı | binder'da bir klasör, mantar pano, anahat | sayfa içe aktarılır (Web Page Import eklentisi, bölüm 14) |
| bir resim dosyası ya da binder'dan bir resim | denetçideki dizin kartı (resim modu) | kartın resmi olur |
| **Proje > Proje Anahtar Sözcükleri** içinden anahtar sözcükler | binder'da bir belge, bir koleksiyon, mantar pano, anahat; denetçinin anahtar sözcük listesi | atanırlar; **Alt** ile: ait oldukları grupla birlikte |

Bazı masaüstleri pencereleri taşımak için Alt+sürükleme kullanır. Alt+sürükleme programa ulaşmıyorsa, masaüstünüzün ayarlarında pencereleri taşımak için kullanılan değiştirici tuşu değiştirin (KDE: Sistem Ayarları, pencere davranışı; tam yer sürümler arasında farklılık gösterir) ya da bunun yerine menü komutlarını kullanın (**Gezin > Aç > Referans Bölmesinde**, **Proje > Proje Anahtar Sözcükleri > Assign to Selected Documents**).

### Hızlı Başvuru panelleri

Bir Hızlı Başvuru paneli, kendi penceresindeki bir belgedir; örneğin yazarken bir karakter sayfasını görünür tutmak için. Bir paneldeki metin, düzenleyicideki gibi düzenlenebilir ve biçimlendirilebilir (Biçim ve Ekle menülerinin kısayolları orada çalışır).

**Panelleri açmak için:**

- Seçili belgeler için **Gezin > Aç > Hızlı Başvuru Olarak** ya da araç çubuğunun Hızlı Başvuru düğmesi.
- Mantar panoda ya da anahatta, ya da **Görünüm > Binder'da Hızlı Ekleme Tuşları** açıkken binder'da ve bir koleksiyonun ya da arama sonucunun listesinde **Space** tuşuna basın (bkz. bölüm 2, binder'ın tuşları). Bu seçenek olmadan, binder'da Space, atlamak için bir adı yazmanın parçasıdır.
- Belgeleri araç çubuğunun **Hızlı Başvuru düğmesine** sürükleyin: her biri kendi panelinde açılır ve düzenleyici belgesini korur.
- **Gezin > Hızlı Başvuruyu Aç** bütün binder'ı bir menü olarak gösterir, böylece bir belgeyi önce seçmeden açabilirsiniz.

**Bir panelde:**

- Başlık çubuğundaki **başlık** düzenlenebilir (belgeyi yeniden adlandırır). Yanındaki **simge** bir menü açar (Binder'da Göster, Düzenleyicide Aç, proje yer imlerine ekle ya da onlardan kaldır) ve belgenin kendisi gibi sürüklenebilir. Bu panelde göstermek için bir belgeyi başlık çubuğuna bırakın.
- **Sağ üstteki** düğme, belgenin **özetini**, **notlarını**, **anahtar sözcüklerini** ya da **üst verisini** metninin altında gösterir. Orada değiştirdiğiniz şey belgeyi değiştirir.
- **Alt bilgi çubuğu** sözcük sayısını gösterir ve **etiketi** ve **durumu** ayarlamanıza izin verir.
- Alt bilgi çubuğunun solundaki **yer imi düğmesi**, belgenin yanında **proje yer imlerini** açar. Yer imli bir belgeye tıklamak onu panelde gösterir, böylece notlarınız arasında dolaşmak için tek bir panel yeterlidir. Yer imlerini başka bir sıraya sürükleyin, belgeleri içine bırakın ve bir yer imini ➖ düğmesiyle kaldırın (belge binder'da kalır). Dosyalar ve web bağlantıları çift tıklamayla açılır.
- Yer imlerinin altındaki ➕ düğmesi **yeni bir belge** oluşturur ve ona yer imi koyar. İlk seferinde klasör sorulur (örneğin Research içinde bir "Notes" klasörü); o andan itibaren o klasörü kullanmak için **Don't ask again** kutusunu işaretleyin. Bunu **Proje > Proje Ayarları > Special Folders** altında değiştirin.
- **Float** düğmesi paneli ana pencerenin üstünde tutar; **Pencere > Hızlı Başvuru Panellerini Ayır** bunu hepsi için yapar.

Pencere menüsü açık panelleri listeler ve hepsini bir kerede kapatır. Bir projeyi kapattığınızda açık olan paneller, projeyi yeniden açtığınızda yerleriyle, boyutlarıyla, alt bölmeleriyle ve kenar çubuklarıyla geri gelir (bunu **Dosya > Seçenekler > Davranışlar** altında kapatın).

### Ses ve video

Binder'daki ses ve video dosyaları (örneğin Research klasöründeki röportajlar), **Media Playback** eklentisi kurulu olduğunda {{APP}}'in içinde oynatılır (bölüm 17; 0.12 güncellemesiyle gelir). O olmadan, böyle bir dosya eskisi gibi **Open with Default Application** gösterir.

**Oynatmak.** Binder'da dosyaya tıklayın. Bir video resmini gösterir; bir ses dosyası başlığını, sanatçısını ve süresini gösterir. Düzenleyicinin **alt bilgi çubuğunda** oynatma denetimleri vardır:

- solda **duraklat ve geri sar** düğmesi (aşağıya bakın),
- **başa git**, **geri atla**, **oynat / duraklat**, **ileri atla**,
- **konum**: kaydırıcıyı sürükleyin ya da tutamacının yanına tıklayın,
- süre, **hız** (0,5× ila 2×; daha yavaş konuşmayı yazmak daha kolaydır; ses, hızla birlikte, bir bant gibi alçalır ya da yükselir) ve **ses düzeyi** (tıklama: sessize al; ok: bir kaydırıcı).

Bir videonun resmine tıklamak onu oynatır ya da duraklatır; sağ tık ayrıca **Open with Default Application** sunar. Klavye oynatıcıdayken: **Space** oynatır ya da duraklatır, **sol** ve **sağ ok tuşları** atlar, **Home** başa gider.

**Yazıya dökmek.** Düzenleyiciyi bölün, ses dosyasını bir düzenleyicide ve yazıya dökümünüzü diğerinde gösterin ve yazmaya devam edin: **Gezin > Medya** komutları klavye nerede olursa olsun, Hızlı Başvuru panellerinde ve yazım modunda da çalışır.

| Komut | Kısayol |
|---------|----------|
| Oynat / Duraklat | Ctrl+Return |
| Geri Atla / İleri Atla (5 saniye) | Ctrl+Alt+, / Ctrl+Alt+. |
| Medya Zaman Damgası Ekle (**Ekle > Medya Zaman Damgası Ekle** de: aynı komut, her iki menüde gösterilir) | Ctrl+Shift+Return |
| Durdur, Başa Git, Duraklat ve Geri Sar, Hızlandır, Yavaşlat, Normal Hız, Sesi Artır, Sesi Azalt, Sessize Al, İmlecin Zaman Damgasına Git | kısayol yok (**Dosya > Seçenekler > Klavye** altında bir kısayol verin) |

Komutlar en son açtığınız ya da kullandığınız dosya üzerinde etki eder (her iki düzenleyicide, bir referans bölmesinde ya da bir Hızlı Başvuru panelinde).

**Duraklat ve geri sar.** Açıkken (düğme basılı görünür), her duraklama biraz geri gider; **Dosya > Seçenekler > Davranışlar > Oynatma** altında başka bir değer ayarlamadıysanız 2 saniye. Duyduğunuzu yazmak için Ctrl+Return tuşlarına bastığınızda, devam ettiğinizde son sözcükleri yeniden duyarsınız.

**Zaman damgaları.** **Medya Zaman Damgası Ekle**, oynatıcının zamanını imlecin olduğu yere yazar, örneğin `[03:07]` (bir saatten itibaren saatlerle: `[1:02:03]`). **İmlecin Zaman Damgasına Git** bunun tersini yapar: imlecin satırındaki zaman damgasını (ondan önceki ya da çevresindeki en yakın olanı) alır ve oynatıcıyı oraya taşır. Zaman damgaları sıradan metindir; elle de yazılabilirler (`3:07`, `[03:07]`, `1:02:03.5`).

**Bir referans bölmesinde ya da bir Hızlı Başvuru panelinde** oynatıcının oynatma denetimleri altında bulunur. Yazıya dökümün düzenleyicisinin referans bölmesindeki bir ses dosyası, yalnızca tek düzenleyici için iyi bir düzendir.

**Kaldığı yer.** Bir ses ya da video dosyasını yeniden açtığınızda, onu duraklattığınız yerden devam eder (bunu **Dosya > Seçenekler > Davranışlar > Oynatma** altında kapatın). Düzenleyicide başka bir belge göstermek dosyayı durdurur.

**Neler oynatılır.** *Media Playback* eklentisi (2.0) kullanımı ücretsiz biçimleri oynatır: MP3, Ogg Vorbis, Opus, FLAC, ALAC, WAV, AIFF ve WavPack ses; VP8, VP9 ya da Theora görüntülü WebM, MKV ve Ogg video (MJPEG ve diğer ücretsiz biçimler de). Eklenti ihtiyaç duyduğu her şeyi getirir.

**Daha fazla biçim.** H.264, H.265 (HEVC), AAC (M4A), AC-3, E-AC-3, DTS, MPEG-1/2/4 video, VC-1 ve WMV, WMA ve AMR bazı ülkelerde patentlerle lisanslıdır; ayrı **Extra Video and Audio Formats** eklentisiyle gelirler. İkisini edinmenin en basit yolu, Releases sayfasındaki tek **media-all-formats.qbpkg** paketidir: her eklenti gibi kurulur (bölüm 17), *Media Playback* ve *Extra Video and Audio Formats* eklentilerini birlikte kurar. İkisi de o zaman **Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi** içinde görünür; orada ek biçimler tek başına kapatılabilir ya da kaldırılabilir. Bu biçimleri kullanmak için bir patent lisansı gerekip gerekmediği ülkeye bağlıdır. Kurulu ve açık olduğu sürece *Media Playback* onu kullanır ve MP4/M4V/MOV, AVI, MPEG, WMV, FLV ve 3GP dosyaları da oynatılır (WMA Pro ve WMV 9 değil). O olmadan, böyle bir dosya oynatıcının altında biçimlerini ve bu eklentiyi adlandıran bir ileti gösterir; diğer her şey eskisi gibi çalışır. AV1 video yerleşik kod çözmeyle oynatılmaz.

0.80.0'dan beri oynatıcı her zaman kendi FFmpeg'iyle kod çözer: {{APP}}'in o zamandan beri kullandığı Qt'nin (The Qt Company'nin kendi Qt 6.12 derlemesi) sistemin **GStreamer**'ı üzerinden oynatma yolu yoktur, bu nedenle **Dosya > Seçenekler > Davranışlar > Oynatma > Kod Çözme** bu seçeneği kullanılamaz olarak gösterir.

### Yer imleri

**Belge yer imleri** (denetçi, yer imi sekmesi) bir belgeyi diğer belgelere, diskinizdeki dosyalara ya da web sayfalarına bağlar. **Proje yer imleri** her belgeden, araç çubuğundaki yer imi düğmesinden ve Hızlı Başvuru panellerinin kenar çubuğundan kullanılabilir. Denetçinin yer imi sekmesi her iki listeyi alt alta gösterir: **Belge Yer İmleri** ve **Proje Yer İmleri**, her birinin kendi ➕ ve ➖ düğmeleri vardır. Bir yer imine tıklamak belgeyi listelerin altında gösterir; düzenleyicinin zaten gösterdiği bir belge orada salt okunur bir kopya olarak görünür (düzenleyicide yazın), böylece düzenleyici satır genişliğini korur. Çift tıklama yer imini açar. Belgeleri, dosyaları ya da web bağlantılarını yer imi listesine sürükleyin ya da ➕ düğmesini kullanın; bir web tarayıcısından sürüklenen bir bağlantı sayfanın başlığını korur. Sırayı değiştirmek için bir yer imini yukarı ya da aşağı sürükleyin. İki liste ile altlarındaki metin arasındaki ayırıcılar taşınabilir; verdiğiniz boyutlar projeyle birlikte saklanır ve yeniden başlatmadan ya da program güncellemesinden sonra, o anda hiçbir yer imi seçili olmasa bile geri gelir. Araç çubuğunun **Yer İmleri düğmesine** bırakılan belgeler proje yer imi olur; **Belgeler > Proje Yer İmlerine Ekle** ile seçili belgeler de öyle olur; denetçi o zaman yer imi sekmesini yeni giriş seçili olarak gösterir.

---

## 7. Denetçi

Denetçinin üstündeki düğmeler şunlar arasında geçiş yapar:

- **Özet ve notlar**: özet, dizin kartında görünen kısa bir özettir. Metin yerine bir resim kullanabilirsiniz: kartın resim düğmesine tıklayın ve gri alana bir resim dosyası ya da binder'dan bir resim bırakın (ya da bir dosya seçmek için alana çift tıklayın; resmi kaldırmak için sağ tıklayın). Altında **belge notları** (ya da küçük menüyle değiştirilen, bütün proje tarafından paylaşılan **proje notları**) bulunur. **Gezin > Denetle > Özete Git** (**Ctrl+Alt+Y**) ve **Belge Notlarına Git** (**Ctrl+Alt+N**) klavyeyi metinden oraya koyar; **Alt+2** düzenleyiciye geri döner. 0.93.0'dan beri notlar (belge ve proje notları) düz metin olarak başlar: projenin metin biçimlendirmesinden yazı tipi, boyut, ilk satır girintisi, paragraf aralığı ya da stil yoktur; notların yakınlaştırmasında programın arayüz yazı tipindedir. Biçimlendirme yalnızca kendiniz uyguladığınızda (biçim çubuğu, menüler, kısayollar) görünür ve sonra kalır. Zaten metni olan notlar sahip oldukları görünümü korur. Notlara yapıştırılan metin düzdür; **Düzenle > Biçimlendirmeyle Yapıştır** (**Ctrl+Alt+Shift+V**) getirdiği biçimlendirmeyi korur.
- **Yer İmleri**: belgenin yer imleri ve projenin yer imleri. Projenin bir belgesinin bir yer imine tıklamak, metnini listelerin altında gösterir. Orada bir düzenleyicide olduğu gibi yazabilir ve biçimlendirebilirsiniz (imleç içindeyken biçim çubuğu, Biçim menüsü ve kısayolları ona etki eder): her şey hemen belgeye ve dosyasına gider ve aynı belgeyi gösteren bir düzenleyici değişikliği hemen gösterir. Oradaki metnin kendi yakınlaştırması vardır.
- **Zihin Haritası**: belge ya da klasör için bir not panosu (aşağıdaki "Zihin haritaları" bölümüne bakın).
- **Nesneler**: hikâyenizin karakterleri, mekânları, ortamları, ilgi çekici noktaları, havası ve diğer şeyleri; her birinin kendi sayfası ve aralarında bağlantılar vardır (aşağıdaki "Nesneler" bölümüne bakın).
- **Üst Veri**: etiket, durum, bölüm türü, derlemeye dahil etme, oluşturulma ve değiştirilme tarihleri, **özel üst veri** ve **anahtar sözcükler**.
- **Anlık Görüntüler** (bölüm 11).
- **Yorumlar ve dipnotlar**.

Eklentilerin yazım araçları kendi sekmelerini ekleyebilir; düğmeleri satırda ilk sırada gelir, örneğin Speech Transcription'ın **Transcriptions** düğmesi (bölüm 10).

**Denetçinin kendi yakınlaştırması.** Denetçinin bir bölümünün üzerinde **Ctrl+fare tekerleği**, düzenleyicileri değiştirmeden o bölümü büyütür ya da küçültür. İki bölümlü sekmelerin her biri için bir yakınlaştırması vardır: **Özet ve notlar** (kart, notlar), **Yer İmleri** (belgenin yer imlerinin listesi, projenin yer imlerinin listesi, altlarındaki yer imli metin: üç yakınlaştırma), **Anlık Görüntüler** (liste, anlık görüntünün metni), **Nesneler** (listeli kategori düğmeleri, sayfa). Alttaki **Yakınlaştırma** listesi, klavyenin bulunduğu bölümün ya da en son yakınlaştırdığınız bölümün yakınlaştırmasını gösterir ve ayarlar; başlığı bölümü adlandırır ("Zoom: Notes") ve Zihin Haritası sekmesinde panonun yakınlaştırmasıdır. Diğer sekmeler (Üst Veri, Yorumlar) ve üstteki ve alttaki düğme satırları tek bir yakınlaştırmayı paylaşır; **Görünüm > Denetçi Sekmelerini Ayrı Yakınlaştır** (Yakınlaştırma listesinin sağ tık menüsünde de) ile Üst Veri ve Yorumlar birer tane tutar.

**Denetçi hatırlar**: projeyle birlikte hangi sekmenin açık olduğunu, sekmelerin içinde ayırıcıları nereye koyduğunuzu, gösterilen yer imini ve Nesneler sekmesinde gösterilen nesneyi.

**Bir klasörün yorumları.** Düzenleyicide bir klasör varken (mantar pano ya da anahat olarak), yorumlar ve dipnotlar sekmesi klasörün ve alt klasörlerinin içindeki her belgenin yorumlarını ve dipnotlarını, belgeye göre gruplanmış olarak listeler. Bir belgenin adına tıklamak onu açar; **Comment ▸** (ya da **Footnote ▸**) üzerine tıklamak belgeyi açar ve o yeri gösterir; bir yorumun ya da dipnotun metni tam orada değiştirilebilir. Birleşik metinde (birkaç belge tek bir metin olarak), sekme gösterilen tüm belgelerin notlarını listeler.

### Zihin haritaları

Her belgenin ve klasörün bir **zihin haritası** olabilir: notları herhangi bir yere koyduğunuz, çizgiler ve oklarla bağladığınız, renklendirdiğiniz ve arka plan şekilleri üzerinde grupladığınız bir pano. Panolar Scapple dosyaları olarak kaydedilebilir ve Scapple dosyaları okunabilir (aşağıdaki *Dosyalar ve Scapple* bölümüne bakın). Denetçinin **Zihin Haritası** düğmesine tıklayın (Yer İmleri ve Üst Veri arasında). Belgenin bir zihin haritası olduğunda düğmenin altı çizilidir.

**Sabitlenmiş bir zihin haritası (0.92.0).** Normalde sekme, seçtiğiniz belgenin ya da klasörün zihin haritasını gösterir. Bunun yerine onu tek bir öğeye sabitleyebilirsiniz: sekme o zaman binder'da ne seçerseniz seçin ya da düzenleyicide ne açarsanız açın o öğenin zihin haritasını göstermeye devam eder. Bu, yazarken yanınızda olmasını istediğiniz bir pano içindir, örneğin bir klasörde tutulan bir olay örgüsü haritası. Üç yer aynı şeyi yapar:

- **Binder'da**, bir satırın üzerine fareyi getirin: sağ ucunun yakınında küçük bir raptiye belirir. Ona tıklayın ve o öğe sabitlenir; raptiyesi kalır ve dolu olur. Dolu raptiyeye tıklamak onu serbest bırakır. Bir raptiyeye tıklamak hiçbir şey seçmez.
- **Zihin Haritası sekmesinde**, küçük çubuğun sağındaki raptiye düğmesi sekmenin gösterdiği panoyu sabitler ve yeniden serbest bırakır. Bir pano sabitlenmişken çubuk onun öğesini adlandırır (*Fixed: Plot*, genel görünüm düğmesinin hemen ardından; uzun bir başlık kısaltılır ve fareyi üzerinde beklettiğinizde başlığın tamamı görünür), böylece panonun seçili belgeye ait olmadığı açıktır.
- **Menülerde**, **Gezin > Denetle > Bu Öğenin Zihin Haritasını Sabitle** ve **Gezin > Denetle > Sabitlenen Zihin Haritasını Bırak** (başlangıçta kısayolları yoktur; isterseniz seçeneklerde onlara bir kısayol verin).

Bir öğe sabitlenmişken, binder'ın altındaki ince bir çubuk başlığıyla birlikte *Mind map fixed:* der ve **Release** düğmesine sahiptir. Bir projenin en fazla bir öğesi sabitlenir; bir başkasını sabitlemek sabitlemeyi taşır. Zihin haritası olmayan bir öğe de sabitlenebilir: sekme başlamak için boş bir pano gösterir. Yalnızca Zihin Haritası sekmesi sabitlenir; denetçinin diğer sekmeleri her zamanki gibi seçili belgeyi izler ve büyük bir pencerede açılan bir pano olduğu gibi kalır. Sabitleme, proje kapatıldığında bu bilgisayarda projeyle birlikte saklanır ve öğe Çöp Kutusuna gittiğinde serbest bırakılır.

**Denetçide** pano sekmeyi doldurur. Üstündeki küçük çubuk **New Note**, **New Background Shape**, **Zoom to Fit**, **Menu** (tüm komutlar: File, Edit, View, Notes, Format), **genel görünüm** düğmesi (bkz. *Etrafa bakmak*) ve **Open in Window** içerir. Altındaki çubuk notları sayar ve yakınlaştırma kaydırıcısını içerir. **Open in Window**, aynı panoyu menü çubuğunda aynı menüler ve sağda bir denetçiyle (**Note Format**, **Note Styles**, **Document Settings**) büyük bir pencerede gösterir; bir yerdeki değişiklikler diğerinde hemen görünür. Panoya sağ tık, farenin altındaki şey için komutları açar. Yeni bir not (düğme, menü ya da çift tıklama) klavyeyi hemen alır, daha önce binder'da olsa bile: yazdığınız şey seçili metninin yerini alır (0.94.0).

Bir zihin haritasının komutları, ana pencerenin menülerinde değil, kendi bu menülerindedir. Aşağıda **Menu > Notes > Connect**, şu anlama gelir: denetçinin panosunun **Menu** düğmesi, sonra **Notes**, sonra **Connect** (büyük pencerede: menü çubuğundaki **Notes > Connect**). Pano klavyeye sahipken, tuşları önce gelir: **Ctrl+Z** panonun bir değişikliğini geri alır, **Ctrl+F** notlarında arar, **Ctrl+P** onu yazdırır.

**Zihin haritaları için yardım.** **Yardım > Zihin Haritası Yardımı** (panonun çubuğunun sağ ucundaki **?** düğmesi ve panonun **Menu > Help** komutu da) resimli kısa bir kılavuz gösterir: notlar nasıl yapılır, bir çizgi nasıl çizilir (bir notu diğerinin üzerine sürükleyin), bir ok nasıl çizilir (bırakırken **Alt** tuşunu basılı tutun; geri ok için **Ctrl+Alt**, iki uç için **Ctrl+Alt+Shift**), çizgi üzerindeki sözcükler ve arka plan şekilleri. **Open Example Board** düğmesi (**Menu > Help > Open Example Board** de) kendi penceresinde, birbirinin üzerine sürüklenecek notlar ve çizgilerin ve okların hazır örnekleriyle bir alıştırma panosu açar. Örnek panodaki hiçbir şey projenize ait değildir ve kaydedilmez.

**Notlar.**

- Boş bir yere **çift tıklayın**: yazmaya hazır yeni bir not. **Ctrl+Enter** seçili notun altında yeni bir not yapar (üst üste). Çift tıklarken **Ctrl** ile, yeni not seçili notlara bağlanır; **Alt** ile ona doğru bir okla; **Ctrl+Alt** ile geri bir okla.
- Metnini değiştirmek için bir nota **çift tıklayın**; **Esc** ya da dışarıda bir tıklama düzenlemeyi bitirir. Kısa bir not metni kadar dar olur; uzun olan yeni notların genişliğinde kırılır (ikisi de **Menu > File > Mind Map Options…** içinde).
- Notları taşımak için **sürükleyin**; bırakırken **Ctrl** ile kopyalanırlar. **Menu > Edit > Arrow Keys Move Notes** (**M**) açıkken **ok tuşları** seçili notları taşır (**Shift** ile 10 px); aksi halde o yönde sonraki notu seçerler. **Enter** ya da **F2** seçili notu düzenler, **Delete** seçili notları kaldırır.
- İkisini bir çizgiyle bağlamak için **bir notu diğerinin üzerine sürükleyin**; çizgiyi kaldırmak için yeniden. **Alt** ile çizgi, üzerine bırakılan nota doğru bir ok alır; **Ctrl+Alt** ile geri bir ok; **Ctrl+Alt+Shift** ile iki ucunda oklar.
- Bir çizgiye bir not koymak için **çizgiye çift tıklayın** ya da değiştirmek için bir çizginin etiketine (**Menu > Notes > Connection Label…**).
- **Menu > Notes > Connect** (**Ctrl+.**), **Connect With Arrow** (**Ctrl+Alt+.**), **Remove Arrows** (**Ctrl+,**) ve **Disconnect** seçili notlar üzerinde çalışır: ilk seçtiğiniz merkezdir.
- **Menu > Notes > New Connected Note**, seçili notun soluna, sağına, üstüne ya da altına ona bağlı (oklu ya da oksuz) bir not yapar.
- **Menu > Notes > Split** (**Ctrl+Shift+K**) bir notu imlecin olduğu yerden böler; **Merge** seçili notları birleştirir.
- **Menu > Notes > Stack** (**Ctrl+'**), **Align**, **Distribute**, **Make Same Width / Height**, **Auto-Fit**, **Bring to Front**, **Send to Back**.
- **Menu > Edit > Add Link…** (**Ctrl+K**) bir notun metnini bir web sayfasına ya da bir dosyaya bağlar; binder'dan panoya sürüklenen belgeler, kendilerine bağlantıları olan notlar olur. Bir bağlantıya tıklamak onu açar: bir belge, metindeki belge bağlantılarının açıldığı gibi açılır (**Dosya > Seçenekler > Davranışlar**).
- **Menu > Notes > Add Notes to the Binder**, seçili notları, panonun ait olduğu belgenin ya da klasörün içinde, panonun okuma sırasına göre belgelere dönüştürür.

**Arka plan şekilleri** (**Menu > Notes > New Background Shape**, **Ctrl+Shift+G**) notların arkasındaki renkli alanlardır. Notlar seçiliyken yapıldığında şekil onları çevreler. Bir **mıknatıslı** şekil (**Menu > Notes > Magnetic**, **Ctrl+Shift+M**) taşındığında üzerindeki notları da götürür. Yeni bir şekil orta gri (#808080) ve mıknatıssız başlar; **Dosya > Seçenekler > Ek Özellikler > Zihin Haritası** (ve **Menu > File > Mind Map Options…**) yeni şekillerin başladığı dolguyu, kenarlığı ve **Magnetic** seçeneğini ayarlar. (0.93.0'dan önce yeni bir şekil açık gri ve mıknatıslıydı; orada seçtiğiniz bir renk kalır ve saklanan bir **Magnetic** bir kez kapatıldı.)

**Bir notun (ya da bir arka plan şeklinin) sağ tık menüsü** seçili notları değiştirir: **Fill Color**, **Text Color** ve **Border Color** projenizin etiketlerinin renklerini, biçim çubuğunun altı rengini (kırmızı, mavi, sarı, yeşil, beyaz, siyah) ve herhangi bir renk için **Other...** seçeneğini sunar; {{APP}}'in her yerinde aynı seçim; **Shape**, **Rectangle**, **Rounded Rectangle**, **Cloud** ya da **Jagged**'dır (kenarlıksız bir nota, şekil görünsün diye ince bir kenarlık verilir); **Border** kalınlığı ayarlar; **Icons** bir nota, her köşeye bir tane ve her kenarın ortasına bir tane olmak üzere, binder'ın simgelerinden (**Belgeler > Simgeyi Değiştir**), kendi simgeleriniz dahil, seçilen en fazla sekiz simge koyar. Yalnızca birini kaldırmak için bir simgenin yerinin yakınında sağ tıklayın; **Remove All Icons** hepsini temizler.

**Bir çizginin sağ tık menüsü** yalnızca o çizgiye aittir, böylece aynı notların diğer çizgileri değişmez: okları (**No Arrows**, her iki nota bir ok ya da **Arrows at Both Ends**), **Line Color**, **Icon in the Middle** (çizginin ortasına çizilen bir simge), etiket, **Insert a Note on the Line** ve **Disconnect**. Her değişiklik tek başına geri alınabilir.

Simgeler, çizgi renkleri ve çizgi simgeleri {{APP}}'in kendi eklemeleridir; pano dosyasında Scapple'ın okumadığı öğelerde saklanırlar (Scapple dosyası olarak dışa aktarılan bir pano notları, çizgileri ve okları bunlar olmadan içerir). **Dosya > Seçenekler > Ek Özellikler > Zihin Haritası** simgelerin boyutunu ve yeni notların biçimini ayarlar.

**Görünüm.** **Menu > Format** kalın, italik, altı çizili, üstü çizili, büyük ve küçük (**Ctrl+Shift+>** ve **Ctrl+Shift+<**), metin, dolgu ve kenarlık renkleri, kenarlık kalınlığı (yok, 1 ila 5 px) ve stili (yuvarlak, kare, tırtıklı, bulut), gölge ve **Fade** (**Ctrl+Shift+F**, not soluk gösterilir) içerir. **Note Styles** bir görünümü bir ad altında saklar; **Menu > Format > Note Style** birini uygular (**Ctrl+Alt+1** ila **9**, Varsayılan için **Ctrl+Alt+0**), seçili nottan bir tane yapar ya da bir notun görünümünü kopyalar ve yapıştırır (**Alt+Shift+C** / **Alt+Shift+V**). Penceredeki denetçinin **Document Settings** bölümü panonun arka planını, metin rengini, yazı tipini ve yazı yönünü ayarlar (ve yeni panolar için **Set As Defaults**).

**Arka plan.** Panonun boş bir yerine sağ tık (denetçide ya da pencerede), **Menu > Format > Background Color** ya da **Document Settings** arka plan düğmesi, projenizin etiketlerinin renklerini, biçim çubuğunun altı rengini, varsayılanı (menülerde **Default for Light Themes** ya da **Default for Dark Themes**, Document Settings içinde **Use Default**) ve **Other...** seçeneğini sunar. Bir panonun açık temalar için kendi arka planı ve koyu temalar için bir tanesi vardır: seçtiğiniz renk kullanımdaki tema türü için saklanır ve diğer tür kendininkini korur. Kendi rengi olmayan bir pano **Dosya > Seçenekler > Ek Özellikler > Zihin Haritası** içindeki varsayılanı ("Background of boards", biri açık biri koyu temalar için) gösterir ve onu değiştirdiğinizde onu izler. Document Settings içindeki **Set As Defaults**, panonun arka planını kullanımdaki tema türünün varsayılanı yapar. Bir pano, üzerinde henüz hiçbir şey olmadığında bile arka plan rengini korur.

**Yakınlaştırma ve yer.** Her pano projeyle birlikte yakınlaştırmasını ve ekrandaki bölümünü hatırlar; biri denetçi için, biri büyük pencere için. Hiç yakınlaştırmadığınız bir pano, zihin haritalarının varsayılan yakınlaştırmasında (**Dosya > Seçenekler > Düzenleme**, "Default text zoom") açılır.

**Seçmek.** Tıklayın, eklemek için **Shift+tıklama** ya da **Ctrl+tıklama**, notların çevresine bir çerçeve sürükleyin, **Ctrl+A**. **Menu > Edit > Select**, altta üst üste yığılmış notları, seçili şekillerin üzerindeki notları (**Alt+A**), bağlı notları (**Ctrl+Shift+A**) ve bütün bağlı kümeleri içerir; altında **Invert Selection** (**Alt+Shift+I**) ve **Deselect** (**Ctrl+D**) bulunur.

**Etrafa bakmak.** **Ctrl+fare tekerleği**, **Ctrl++** ve **Ctrl+-**, **Actual Size** (**Ctrl+0**), **Zoom to Fit** (**Ctrl+9**), **Zoom to Selection** (**Ctrl+Shift+8**) ve **Zoom to Random** (**Ctrl+Shift+R**) ile yakınlaştırın. Panoyu taşımak için **Space** tuşunu basılı tutup sürükleyin; panonun tamamını bir an görmek için **Z** tuşunu basılı tutun. **Menu > Edit > Find** (**Ctrl+F**) notlardaki metni bulur ve değiştirir. **Menu > View > Page Guides** yazdırırken sayfaların nerede bittiğini gösterir.

**Genel görünüm (0.93.0).** Panonun sol üst köşesindeki küçük bir harita, panonun tamamını küçük olarak ve şu an gördüğünüz bölüm için bir çerçeve gösterir; çerçeve siz kaydırdıkça ve yakınlaştırdıkça hareket eder ve boyutunu değiştirir. Notlar, arka plan şekilleri ve çizgiler renkleriyle çizilir; bir notun metni yalnızca o boyutta hâlâ okunabildiği yerde gösterilir, aksi halde not renkli çerçevesidir. Görünümü o noktaya taşımak için genel görünüme tıklayın; görünümü onunla birlikte taşımak için çerçeveyi sürükleyin. Yakınlaştırma kalır ve kilitli bir pano (**Prevent Editing**) da gezdirilebilir. Genel görünümün üzerindeki fare tekerleği panonun üzerindeki gibi çalışır (**Ctrl+tekerlek** yakınlaştırır). Genişliğini değiştirmek için sağ alt köşesindeki tutamacı sürükleyin (görünümün genişliğinin beşte biriyle, en az 80 px olarak başlar ve hiçbir zaman görünümün yarısından geniş olmaz); yükseklik gösterdiği şeyin biçimini izler, hiçbir zaman görünümün yarısından fazla olmaz ve yükseklik bunu aşacağında genişlik onunla birlikte küçülür. Genel görünüm yalnızca görünüme aittir: dışa aktarmalarda, baskılarda ya da panonun resminde yoktur. Başlangıçta **otomatiktir**: panonun bir bölümü görünümün dışında kaldığında görünür ve hepsi sığdığında gizlenir (kenarda titremesin diye küçük bir marjla). Denetçinin panosunun çubuğundaki **genel görünüm** düğmesi (**Menu**'nün hemen ardından) **otomatik** ve **her zaman gösterilir** arasında geçiş yapar (düğme basılı olur); aynı anahtar panonun Görünüm menüsündeki **Overview Always Shown** girişidir (denetçide: **Menu** düğmesi, sonra **View**; büyük pencerede: menü çubuğu). Seçim ve boyutlar bu bilgisayarda, programın ayarlarında saklanır (tüm panolar için tek bir seçim; denetçinin genel görünümünün ve büyük pencerenin boyutu ayrı); projeye hiçbir şey girmez.

**Metin, resimler ve dosyalar.** Bir not yapmak için metni yapıştırın ya da bırakın (boş satırlı metin birkaç not olabilir), bir resim notu için bir resim bırakın ve paragraflarından notlar yapmak için bir metin dosyası bırakın.
**Dosyalar ve Scapple.** **Menu > File > Export**, panoyu PDF (**Ctrl+Shift+E**), PNG resim, düz metin, metin listesi, RTF, OPML anahattı, resimleri ya da bir Scapple dosyası (*.scap*) olarak yazar; **Menu > File > Import Mind Map or Scapple File…** bir *.scap* dosyasını panoya okur. **Print** ve **Print Preview** onu yazdırır. **Menu > File > Prevent Editing** panoyu kilitler.

**Mind Map Options** (**Menu > File > Mind Map Options…** ve ana pencerenin **Dosya > Seçenekler > Ek Özellikler > Zihin Haritası** altında aynısı): **Enter** tuşunun bir notun düzenlemesini bitirip bitirmeyeceği (o zaman **Shift+Enter** yeni bir satır yapar), seçili bir notta **Enter** tuşunun yeni bir not yapıp yapmayacağı, yeni notların seçili notun görünümünü alıp almayacağı, oksuz kesikli çizgiler, kısa metinler için dar notlar, soluk notların ne kadar soluk olduğu ve yeni notların ve şekillerin boyutu, genişliği, rengi ve kenarlığı.

**Panonun saklandığı yer.** Bir pano kendiliğinden, Scapple dosyalarının düzeninde bir dosya olarak kaydedilir (proje klasöründe *content/<ID>/mindmap.scap*; bir Scrivener projesinde belgenin kendi dosyalarının yanında *Files/Data/<UUID>/quillbinder-mindmap.scap*). Bir belge dosyayı yalnızca panosuna bir şey konduğunda alır. Geri Al (**Ctrl+Z**) her pano için ayrı ayrı çalışır.

{{APP}}, Scapple dosyalarını Scapple 1.x'in yazdığı gibi, düzenleri bilindiği ölçüde okur; bunları Scapple'ın kendisiyle sınamak mümkün olmadı (durum dosyasında 0.17.0 sürümünün notlarına bakın).

### Nesneler: karakterler, mekânlar ve daha fazlası

**Nesneler**, hikâyenizin insanları, yerleri ve şeyleridir; her birinin bir alanlar sayfası vardır: ırkı, boyu, göz rengi, özellikleri ve geçmiş hikâyesi olan bir karakter; türü, ortamı ve havası olan bir yer; ve benzerleri. Bütün projeye aittirler, tek bir belgeye değil ve binder'dan ayrı tutulurlar. Bağlantılar onları birbirine bağlar, böylece bir arka planda tıklayarak ilerleyebilirsiniz: karakter sayfası, onun yaşadığı köy, çevresindeki manzara, havası.

Denetçinin **Nesneler** düğmesine tıklayın (**Zihin Haritası** düğmesinin sağında) ya da **Gezin > Denetle > Projenin nesneleri: karakterler, mekânlar, ortamlar, ...** seçin. **Pencere > Nesneler…** (ya da sekmenin çubuğundaki pencere düğmesi) aynı nesneleri solda liste, sağda sayfa olmak üzere büyük, kendi penceresinde gösterir. Bir yerdeki değişiklikler diğerinde hemen görünür.

**Kategoriler.** Üstteki renkli düğmeler kategorilerdir: başlangıç olarak **Characters**, **Locations**, **Environments**, **Points of Interest** ve **Weather**. Nesnelerini görmek için birine tıklayın. Onları yeniden adlandırabilir, başka renkler verebilir, sıralarını değiştirebilir, kaldırabilir ve kendi kategorilerinizi ekleyebilirsiniz (gruplar, büyüler, gemiler ...): aşağıdaki *Şablonlar ve renkler* bölümüne bakın.

**Nesne yapmak.** Sekmenin çubuğundaki **+**, seçili kategoride (seçili nesnenin klasöründe) yeni bir nesne yapar; adını sayfanın üstüne yazın. Klasörlü düğme sıralamak için bir **klasör** yapar, örneğin Locations içinde "Villages"; klasörler klasör tutabilir. Nesneleri ve klasörleri taşımak için listede sürükleyin. **F2** ya da sağ tık yeniden adlandırır ve sağ tık menüsünde ayrıca **Duplicate** ve **Delete…** vardır (çubuğun **≡** menüsünde de). **Filter** alanı yalnızca adı ya da metni yazdığınızı içeren nesneleri gösterir.

**Sayfa.** Kategori ve adın bulunduğu renkli şeridin altında, kategorinin şablonunun alanları vardır. Yazdığınız her şey, belgeleriniz gibi kendiliğinden kaydedilir. Alan türleri şunlardır:

- **Short text** (tek satır) ve **Date** (istediğiniz biçimde, örneğin "spring 1203" ya da "the third winter").
- Bir birimle **Number** (cm, yıl, °C ...).
- Bağlantılar (aşağıya bakın) ve **kalın**, *italik*, altı çizili, üstü çizili, renkli ve vurgulu sözcükler tutabilen **Long text**: imleç alandayken biçim çubuğu, Biçim menüsü ve kısayolları (**Ctrl+B**, **Ctrl+I**, **Ctrl+U**; Nesneler penceresinde de) ona etki eder. {{APP}}'in 0.20 ve 0.21 sürümleri bunları da gösterir.
- **Choice**: seçilecek bir liste; başka bir şey de yazabilirsiniz.
- **Yes / no**.
- **Picture**: birini seçmek için alana tıklayın (ya da üzerine bir resim bırakın); projeye kopyalanır, 1600 pikselden büyükse küçültülür. Sağ tık onu kaldırır.
- **Link to objects**: bağlı nesneler renkli düğmeler olarak gösterilir. **+** bir tane ekler: aramak için yazın, sonra bir nesne seçin ya da o adla yeni bir nesne yapıp hemen bağlamak için **New in Locations: "Ashvale"** seçin. Bağlı bir nesneye tıklamak sayfasını gösterir; sağ tık bağlantıyı kaldırır.
- **Link to documents**: bir karakterin "Appears in" ya da bir yerin "Scenes set here" alanları gibi, nesneye ait binder belgeleri. Bir tıklama belgeyi düzenleyicide açar.

**Renk düğmesi.** Şeridin **Color** düğmesi bir nesneye kendi rengini (ya da kategorinin rengini geri) verir, örneğin ana karakterleri işaretlemek için.

**Uzun metinlerin içindeki bağlantılar.** Uzun bir metin alanında bir sözcüğü seçin ve sağ tık menüsünden **Nesneye Bağlantı…** (ya da **Belgeye Bağlantı…**) seçin; seçim yoksa nesnenin adı bir bağlantı olarak konur. Bağlantıya tıklamak onu izler; sağ tık menüsündeki **Remove Link** onu yeniden kaldırır. Bir belgenin metninde **Ekle > Nesneye Bağlantı…** metne bir nesneye bağlantı koyar; ona tıklamak nesneyi denetçide gösterir. Müsvedde derlendiğinde bu bağlantılar sıradan sözcüklere dönüşür.

**İleri geri gitmek.** Sekmenin çubuğundaki **‹** ve **›**, bir web tarayıcısı gibi, baktığınız nesneler arasında geri ve ileri gider.

**Bağlantı verenler.** Her sayfanın altında **Linked from** bu nesneye bağlantı veren nesneleri (bir bağlantı alanında ya da bir metinde) listeler, böylece köy kimin içinde yaşadığını gösterir. Bir tıklama oraya gider.

**Bu belgede.** Düzenleyicideki belge nesnelere bağlıysa (örneğin "Appears in" gibi bir belge bağlantısı alanı aracılığıyla), sekme bu nesnelerle birlikte **In this document:** gösterir ve **Nesneler** düğmesinin altı, içeriği olan diğer sekmeler gibi çizilir.

**Silmek.** Bir nesneyi silmek önce sorar. Ona olan bağlantılar diğer nesnelerden kaldırılır (metinlerde bağlı sözcük düz metin olarak kalır). Bir belgedeki ona bir bağlantı, tıkladığınızda nesnenin artık var olmadığını söyler. Bir klasörü silmek nesnelerini korur; bir düzey yukarı çıkarlar.

**Şablonlar ve renkler.** **Proje > Nesne Şablonları ve Renkleri…** (sekmenin **≡** menüsünde ve bir kategori düğmesinin sağ tık menüsünde de) kategorileri ve alanlarını düzenler:

- Solda kategoriler: **Add**, **Kaldır** ve sıra için oklar. Nesneleri olan bir kategoriyi kaldırmak, onların da silinip silinmeyeceğini sorar; **No** hiçbir şeyi değiştirmez.
- Sağda kategorinin adı ve **Color** rengi ve alanları: **Add Field**, **Kaldır**, **Up**, **Down**. Her alan için adı ve türü; bir sayının birimi, bir seçimin seçenekler listesi (her satırda bir) ve bir bağlantı alanının bağlanabileceği kategoriler olabilir (hiçbiri işaretli değilse: hepsi).
- **Use for New Projects** bu kategorileri ve şablonları bundan sonra yapacağınız projeler için saklar. **Standard Templates** bir kategoriyi geldiği şablona sıfırlar ya da kaldırdığınız standart kategorileri ekler.
- Bir alanı şablondan kaldırmak, nesnelerin içinde sahip olduklarını gizler, ancak dosyalarından silmez. Standart bir alan için **Standard Templates > Reset This Category to the Standard** onu içeriğiyle geri getirir; kendi alanınız geri gelmez, bu nedenle içeriğini yeniden isteyebilecekseniz bir alanı kaldırmak yerine yeniden adlandırın.

**Nesnelerin saklandığı yer.** Proje klasöründe, *objects* klasöründe: nesne başına bir küçük dosya (*objects/characters/<ID>.json* ve benzerleri), şablonlar, renkler ve klasörler *objects/schema.json* içinde ve resimler *objects/images* içinde. Proje dosyasının kendisi değişmez, böylece {{APP}}'in eski sürümleri projeyi eskisi gibi açar ve nesnelere dokunmaz. Scrivener biçimindeki bir proje nesnelerini çalışma kopyasında tutar (bölüm 14); Scrivener onları bilmez. Yedekler onları içerir.

---

## 8. Projenizi düzenlemek

### Anahtar sözcükler

Anahtar sözcükler etiketlerdir, örneğin bir sahnedeki karakterler ya da yerler. Onları denetçide ekleyin ya da hepsini **Proje > Proje Anahtar Sözcükleri** (**Ctrl+Alt+K**) ile yönetin; orada onlara renk verebilir, seçili belgelere atayabilir ve bir anahtar sözcüğü olan tüm belgeleri bulabilirsiniz. Anahtar sözcükleri o pencereden binder'daki belgelere, bir koleksiyona, mantar panoya ya da anahatta ya da denetçinin ya da bir Hızlı Başvuru panelinin anahtar sözcük listesine **sürükleyebilirsiniz** (belge seçili birkaç belgeden biriyse hepsi anahtar sözcükleri alır). Bir anahtar sözcüğün grubunu da atamak için bırakırken **Alt** tuşunu basılı tutun. Pencerenin içinde, bir anahtar sözcüğü o gruba koymak için başka birinin üzerine sürükleyin. Anahtar sözcük renkleri dizin kartlarında görünür.

### Özel üst veri

**Proje > Proje Ayarları > Özel Üst Veri** altında kendi alanlarınızı ekleyebilirsiniz: metin, onay kutusu, liste (sabit bir seçenek kümesi) ya da tarih; örneğin *POV*, *Location* ya da *Story Day*. Denetçide ve anahat sütunları olarak görünürler.

### Projenin ayarlarını bir dosyaya kaydetmek

**Proje > Proje Ayarlarını Dosyaya Kaydet…** açık projenin ayarlarını kendi dosyasına (*.qbsettings*) yazar: etiketler, durum, bölüm türleri, özel üst veri, anahtar sözcükler, varsayılan biçimlendirme, stiller, senaryo biçimi, hedefler, derleme ayarları, sayfa görünümü ve sayfa boyutu, yazdırma ayarları, görünüm (tema, renkler, yazı tipleri, sütun genişliği, kenarlık), projenin kendi seçenekleri (Dosya > Seçenekler, "This project" ile) ve Nesneler sayfasının şablonları ve renkleri. Belgelerinizin metni buna dahil değildir. Etiketler, bölüm türleri ve derleme ayarlarının adlandırdığı klasörler adlarıyla saklanır, böylece dosya başka bir projeye okunabilir.

**Proje > Proje Ayarlarını Dosyadan Yükle…** böyle bir dosyayı açık projeye okur. Projenin ayarları dosyadakilerle aynıysa, bunu söyler ve hiçbir şeyi değiştirmez. Farklıysa, bir pencere her bölümü neyin farklı olduğuyla birlikte listeler (örneğin "only in the file: Idea; only in the project: Revise") ve her bölüm için seçmenize izin verir:

- **Take from the File**: bu bölüm için dosyanın ayarları.
- **Keep the Project's**: bölüm olduğu gibi kalır.
- **Quillbinder Default**: yeni bir projenin aldığı (**Dosya > Seçenekler**'in yeni projeler için ayarları, standart etiketler ve durumlar ...).

**Take All from the File** ve **Keep All of the Project's** her bölümü bir kerede ayarlar. Ne seçerseniz seçin, belgelerinizin kullandığı hiçbir şey kaybolmaz: belgelerin hâlâ kullandığı bir etiket, durum, bölüm türü, anahtar sözcük, özel üst veri alanı, nesne kategorisi ya da stil, dosyada olmasa bile projede kalır ve sondaki ileti nelerin tutulduğunu adlandırır. Biçimlendirme ya da bir stil değiştiğinde, kendiniz değiştirdiğinizde olduğu gibi tüm belgelerin metni onu izler. Yüklemeden önce {{APP}}, ayarları olduğu gibi projenin *settings* klasöründeki bir dosyada ("settings-before-loading-*date*.qbsettings") saklar; geri dönmek için o dosyayı yükleyin.

### Bölüm türleri

Bölüm türleri Derle'ye her belgenin ne olduğunu söyler: *Part*, *Chapter*, *Scene*, *Section* ve benzerleri. Onları denetçide belge başına ayarlayın ya da {{APP}}'in konuma göre karar vermesine izin verin (**Proje Ayarları > Bölüm Türleri**, "Default types by structure"), örneğin: ilk düzeydeki klasörler bölümlerdir, içlerindeki metin belgeleri sahnelerdir.

### Belge şablonları

Şablon belgeleri (örneğin bir karakter sayfası) bir klasöre koyun, sonra o klasörü **Proje Ayarları > Special Folders** altında seçin. Şablonlar **Proje > Şablondan Yeni** altında görünür. **Belgeler > Alt Belgeler İçin Varsayılan Şablon** altında bir klasörün, içinde oluşturduğunuz her yeni belge için bir şablon kullanmasını sağlayabilirsiniz.

### Proje şablonları

**Dosya > Şablon Olarak Kaydet…** geçerli projeyi (yapı, ayarlar ve örnek belgeler) yeni projeler için bir şablon olarak kaydeder.

---

## 9. Arama

### Hızlı arama

Araç çubuğundaki alana birkaç harf yazın. Bir liste, **başlığı**, **özeti** ya da **metni** bunları içeren belgeleri, her tür kendi başlığı altında olmak üzere en fazla on tane gösterir. Birini açmak için seçin: metinde yazdığınızı içeren yerler **işaretlenir** ve ilki seçilir ve görünüme kaydırılır. İşaretler, binder'da başka bir belge seçtiğinizde kaybolur. Bu, adını bildiğiniz bir belgeye giden en hızlı yoldur.

### Proje araması

**Ctrl+Shift+F** ya da araç çubuğundaki büyüteç, binder'ın üstünde bir arama alanı gösterir. Yazın ve binder'ın yerini mor sonuç listesi alır.

- Bir sonucu açmak için tıklayın. **Eşleşmeler metinde vurgulanır**, ilki seçilir ve görünüme kaydırılır ve **F3** / **Shift+F3** birinden diğerine atlar. **Ctrl+F** bul çubuğunu arama teriminizle önceden doldurulmuş olarak açar.
- Sonuçların başlığındaki **X** binder'a döner; düzenleyici gösterdiği belgeyi korur. Arama alanını gizlemek için araç çubuğundaki büyüteçe yeniden tıklayın.
- Koleksiyonlar gösterilirken, **Search Results** sekmesi son aramanızı yeniden çalıştırır. Son arama projeyle birlikte saklanır.

Arama alanının solundaki büyüteç seçenekleri açar:

| Grup | Girişler |
|-------|---------|
| **Search In** | **All** ya da şunlardan biri: Title, Text, Synopsis, Notes, Keywords, Label, Status, Custom Metadata. Bir tıklama yalnızca o yeri seçer. Birkaç yer seçmek için tıklarken (ya da menüyü açarken) **Alt** tuşunu basılı tutun. |
| **Operator** | **All Words** (her sözcük, herhangi bir sırada geçmelidir), **Any Word**, **Exact Phrase**, **Regular Expression** |
| **Options** | Whole Words, Case Sensitive, Invert Results (eşleşmeyen belgeler), Search Draft Only, Search Binder Selection Only (binder'da seçili belgeler ve içlerindeki her şey), Search "Included" Documents / Search "Excluded" Documents (**Derlemeye Dahil Et**'e göre), Include Trash |

Örnek: hâlâ "To Do" durumunda olan her şeyi görmek için `To Do` arayın ve Search In altında **Status** seçin.

Menünün altındaki **Save Search as Collection…** aramayı, seçenekleriyle birlikte, her açtığınızda yeniden çalışan bir koleksiyon olarak saklar.

### Projede Değiştir

**Düzenle > Bul > Projede Değiştir** bir metni bütün projede bir kerede değiştirir.

1. **Replace** edilecek metni ve onun yerine konacak metni (**With**) girin. Metni kaldırmak için "With" alanını boş bırakın.
2. **Match case**, **Whole words only** ya da **Regular expression** seçin (o zaman "With" ifadenin grupları için `\1` ila `\9` kullanabilir).
3. Nerede değiştirileceğini işaretleyin: **Titles**, **Text**, **Notes**, **Synopses**, **Custom metadata** (metin alanları), **Comments and footnotes** (denetçide gösterilen metinler).
4. **Entire project** ya da **Selected documents only** (binder'da seçili belgeler, içlerindekilerle birlikte) seçin. İşaretlemediğiniz sürece Çöp Kutusuna dokunulmaz.
5. **Count** kaç yerin değişeceğini ve nerede olduğunu söyler. **Replace All** bir kez daha sorar ve sonra yapar.

Projede Değiştir bir bütün olarak geri alınamaz. **Take a snapshot of every text before it is changed** kutusunu işaretli bırakın: değişen her metin o zaman geri dönülecek "Before replacing …" adında bir anlık görüntüye sahip olur (bölüm 11). Başlıkların ve özetlerin anlık görüntüsü yoktur; büyük bir değişiklik için önce **Dosya > Yedekle > Şimdi Yedekle** ile bir yedek alın. Bir düzenleyicide açık olan bir metin, orada **Düzenle > Geri Al** ile de geri alınabilir.

Biçimlendirme olduğu gibi kalır: yeni metin, yerini aldığı metnin biçimlendirmesini devralır. Bir düzenleyicide açık olmayan bir metin kendi dosyasında değiştirilir, bu nedenle **Düzenle > Geri Al** bu değişikliği geri almaz (önce alınan anlık görüntü alır); aynısı Tırnakları Dönüştür, yorum kartları ve Karşılaştır ve Aktar için de geçerlidir.

### Biçimlendirmeye Göre Bul

**Düzenle > Bul > Biçimlendirmeye Göre Bul** (**Ctrl+F3**) siz çalışırken açık kalan küçük bir pencere açar. Ne arayacağınızı seçin:

| Bul | Bulduğu | Ek seçenek |
|------|-------|--------------|
| Highlighted Text | vurgulayıcıyla işaretlenmiş metin | tek bir renkle sınırla |
| Comments and Footnotes | bir yorumu ya da dipnotu eklenmiş metin (denetçi) | tek bir yorum rengiyle sınırla |
| Inline Annotations, Inline Footnotes | metne yazılan notlar | açıklama rengi |
| Revision Color | revizyon modunda yazılan metin | hangi revizyon |
| Colored Text | rengini elle ayarladığınız metin | tek bir renkle sınırla |
| Links | web bağlantıları ve belgelere bağlantılar | |
| Character Format | kalın, italik, altı çizili, üstü çizili metin (elle ayarlanmış, bir stille değil) | hangisi |
| Style | bir paragraf ya da karakter stilindeki metin | hangi stil |
| Images | metindeki resimler | |

**Containing text** aramayı bir sözcük içeren yerlerle daraltır (yorumlar ve dipnotlar için notun kendi metni de sayılır). **Next** ve **Previous** sonraki yeri seçer. **Search in: All Documents** ile arama binder'daki sonraki belgelerde devam eder ve onları açar; **The Current Editor Only** ile bulunduğunuz yerde kalır. **Ctrl+G** ve **Ctrl+Shift+G** aramayı pencere olmadan yineler. **Close** (ya da **Esc**) pencereyi kapatır.

---

## 10. Hedefler, istatistikler ve yazım araçları

- **Proje > Proje Hedefleri** (**Ctrl+Shift+T**): Taslak (Draft) için ve her yazma oturumu için bir sözcük ya da karakter hedefi belirleyebilir, isterseniz bir son tarih ve yazma günleri de girebilirsiniz. {{APP}} bu durumda her oturumda ne kadar yazmanız gerektiğini hesaplayabilir. Alt bilgi çubuğu, sözcük sayısının yanında ilerlemenizi gösterir (0.94.0'dan beri): oturum için küçük bir çubuk ve örneğin "83 / 1,000 bugün", Taslak için "12,400 / 80,000 taslak"; her biri yalnızca hedefi belirlendiği sürece görünür ve üzerine tıklamak bu pencereyi açar. Hızlı arama kutusunun altındaki iki ince çubuk (mavi: Taslak; turuncu: oturum) yerinde durur ve **Dosya > Seçenekler > Dış Görünüm > Genel Arayüz** bölümünden kapatılabilir. Projeye tek adımda giren sözcükler oturumda yazılmış sayılmaz (0.94.0): içe aktarılan metinler (**Dosya > İçe Aktar**, **İçe Aktar ve Böl**, bırakılan dosyalar, web sayfaları), 200 sözcüğü aşan yapıştırmalar ve bir projeyi ilk kez açtığınızda içinde zaten bulunan sözcükler. **Oturumu Şimdi Sıfırla** sayımı 0'dan başlatır; hızlı arama kutusunun ipucu bunu hemen gösterir. Hedefler penceresi çubukları için yer ayırır; bu yüzden bir hedef belirlediğinizde alanlar yer değiştirmez.
- **Belge hedefleri**: düzenleyicinin alt bilgisindeki hedefe tıklayın.
- **Kesme hedefleri** (0.95.0): bir hedef negatif bir sayı olabilir, her hedef alanında (Taslak, oturum ve belge). Bu, kesmek için bir hedeftir: bir oturum için "-2,000" bugün 2.000 sözcük kesmek, bir belge için "-4,000" onu 4.000 sözcüğe indirmek demektir. Çubuğu sayım düştükçe dolar (oturum çubuğu çıkarılan sözcüklerle, belge ya da Taslak çubuğu sayım o sayıya inene kadar) ve negatif bir sayı kırmızı gösterilir. Negatif bir oturum hedefiyle bugünkü sayım nettir: çıkarılan sözcükleri 0'ın altında, kırmızı bir sayı olarak sayar; pozitif bir hedefle eskisi gibi kalır ve hiçbir zaman 0'ın altına inmez. Scrivener negatif hedefleri kabul etmez: Scrivener biçimli bir projede *.scriv* bunlar için "hedef yok" tutar ve {{APP}} sayıyı kendi çalışma kopyasında saklar.
- **Proje > Proje İstatistikleri**: Taslakta, derlenen metinde ve tüm projede sözcük ve karakter sayıları ile tahmini sayfa sayısı. Büyük bir projede sayım biraz sürer; meşgul imleci bunu gösterir.
- **Proje > Yazma Geçmişi**: her gün kaç sözcük yazdığınız.

Aşağıdaki yazım araçları birer eklentidir (Bölüm 17): **İsim Üretici**, **Karalama Defteri** ve **Metin İstatistikleri** (0.11 güncellemesiyle tek paket olarak geldiler) ile **Konuşma Yazıya Dökme** (0.61); her biri **Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi** bölümünden kapatılabilir ya da kaldırılabilir. Bunlar yoksa **Düzenle > Yazım Araçları** bunu söyler; Eklenti Yöneticisi Yardım menüsündedir.

### Metin İstatistikleri

**Proje > Metin İstatistikleri…** projenizin bir bölümünü sayar. Hangi metnin sayılacağını en üstten seçin:

- **Seçili metin** (pencereyi metin seçiliyken açtığınızda seçilir),
- **Düzenleyicideki metin** (birleşik metinde: gösterilen tüm belgeler),
- **Binder'da seçili belgeler**, içlerindeki belgelerle birlikte,
- **Taslak (derlemeye dahil)** ya da **Tüm proje** (Çöp Kutusu hariç).

Pencerede dört sekme vardır:

- **Genel Bakış**: sözcükler (alt bilgi çubuğu ve hedeflerle tamamen aynı şekilde sayılır), boşluklu ve boşluksuz karakterler, paragraflar, cümleler, farklı sözcükler, 250 sözcüklük sayfalarla sayfa sayısı, dakikada 230 sözcükle okuma süresi ve ortalamalar (sözcük başına harf, cümle ve paragraf başına sözcük, en uzun cümle; okumak için üzerine gelin).
- **Okunabilirlik (tahmini)**, aynı zamanda ilk sekmede. İngilizce için: *Flesch Reading Ease* (0 ile 100 arası, yüksek değer daha kolay demektir; 60 ile 70 arası sade İngilizcedir) ve *Flesch-Kincaid Grade Level* (metni izlemek için gereken ABD okul sınıfı). Almanca için: *Amstad'ın Flesch indeksi* (aynı ölçek) ve *Wiener Sachtextformel* (yaklaşık okul sınıfı; 4 kolay, 15 çok zor). Her dil için: *LIX* (cümle başına sözcük artı altı harften uzun sözcüklerin payı; 30'un altı çok kolay, 60'ın üstü çok zor). Dil kendiliğinden bulunur ya da en üstten seçilir. Bu puanlar birer kılavuzdur, hüküm değildir: heceler ve cümleler pratik kurallarla bulunur ve formüller diyalog ya da şiir için değil, bilgi aktaran düz yazı için geliştirilmiştir.
- **Sözcük Sıklığı**: her sözcük, sayısı ve payıyla, en sık geçenden başlayarak. **Yaygın sözcükleri gizle** seçeneği artikelleri, zamirleri, edatları ve benzerlerini dışarıda bırakır (İngilizce ve Almanca). En az uzunluk belirleyebilir, bir sözcük arayabilir ya da büyük harfleri ayrı sayabilirsiniz. Bir sözcüğe çift tıklarsanız proje içinde o sözcük aranır.
- **Tekrarlanan Öbekler**: cümle içinde birden fazla kullandığınız iki ila altı sözcüklük gruplar; örneğin iki kez verdiğiniz bir betimleme. Yalnızca yaygın sözcüklerden oluşan öbekler dışarıda bırakılır. Projede aramak için çift tıklayın.
- **Belgeler**: belgelerin sözcük, karakter, cümle, cümle başına sözcük sayıları ve bütündeki paylarıyla bir tablosu. Sıralamak için bir sütun başlığına tıklayın; bir belgeyi açmak için çift tıklayın.

**Tabloyu Dışa Aktar…** geçerli sekmenin tablosunu bir e-tablo programı için CSV dosyası olarak kaydeder.

### İsim Üretici

**Düzenle > Yazım Araçları > İsim Üretici…** karakterler için isimler üretir.

1. **Ön adlar** için listeleri işaretleyin (Avrupa, Amerika, Asya ve Afrika'dan 62 köken; örneğin İngiliz, Amerikan, İrlandalı ve İrlanda Galcesi, Alman, Avusturyalı, İsviçreli, Hollandalı, Fransız, Fransız Kanadalı, İtalyan, İspanyol, Meksikalı, Portekizli, Brezilyalı, Danimarkalı, İsveçli, Norveçli, Fin, İzlandalı, Polonyalı, Çek, Macar, Rus, Yunan, Türk, İbrani, Arap, Fars, Hint, Japon, Çinli, Koreli, Yoruba, Svahili ve Zulu; henüz İskoç ya da Galli listesi yoktur). **Soyadları** ön adla aynı listeden gelir; ya da **Ön adın listesi** işaretini kaldırıp başka listeler seçebilirsiniz (İrlandalı soyadlı bir İtalyan ön adı gibi).
2. **Cinsiyet**, **İsim sayısı** ve neyin **gösterileceğini** (tam ad, yalnızca ön ad, yalnızca soyadı, baş harfler ve soyadı) seçin. **İkinci adla** ikinci bir ön ad ekler.
3. **İnce ayar**: **yaygın ↔ nadir** en yaygın isimlerden (sol) ya da nadir olanlardan (sağ) seçer; bu, her ismin ne kadar sık geçtiğini taşıyan, *sıklığa göre* işaretli listelerle çalışır (örneğin Amerikan, İsveç, Macar, İbrani). **Aliterasyon** isimlerin bu oranına ön adın ilk harfiyle başlayan bir soyadı verir (Peter Parker); **Çift soyadı** iki soyadını kısa çizgiyle birleştirir. **Ön ad şununla başlar** / **Soyadı şununla başlar** isimleri bu harflerle başlayanlarla sınırlar.
4. **İsimleri Üret** düğmesine tıklayın. İsimleri seçip **Kopyala** (hiçbiri seçili değilse tüm isimler) ya da **Metne Ekle** (çift tıklayarak da) düğmesine tıklarsanız isimler düzenleyicinin imlecine konur.

Bir dilde soyadının kadın biçimi varsa kadınlara o biçim verilir (Lehçe *Kowalska*, Çekçe *Nováková*, Rusça *Иванова*). Japon, Çin, Kore, Macar ve Vietnam isimlerinde önce soyadı gösterilir. Bazı listeler kendi yazısıyla yazılmıştır (Rusça, Yunanca, İbranice, Arapça ve diğerleri); listedeki adlar hangisi olduğunu söyler.

**Kendi Listemi Ekle…** her satırında bir isim bulunan bir metin dosyasını kendi listenize (ön adlar, kadın ya da erkek ön adları ya da soyadları) dönüştürür. Listeyi seçip **Kendi Listemi Kaldır** düğmesine tıklarsanız silinir. İsim listeleri açık kaynaklı *Faker* projesinden (MIT lisansı) gelir; bunlar kullanımda olan isimlerin listeleridir, denetlenmiş tarihsel isim listeleri değildir.

### Karalama Defteri

Karalama defteri, her projenin dışında kalan notları tutar: başka bir proje açıkken aklınıza gelen fikirler, bir web sayfasından kopyalanan alıntılar, ekranın bir bölümünün resmi. **Pencere > Karalama Defteri** (**Alt+Shift+Return**) ile açın.

- **+** yeni bir not başlatır (yanındaki ok: biçimlendirilmiş ya da düz metin). Metnin üstündeki başlık notun adıdır. **−** seçili notu siler.
- Biçimlendirilmiş notlarda **kalın**, *italik* ve altı çizili metin (düğmeler ya da **Ctrl+B / I / U**) ve resim kullanılabilir: bir resim yapıştırın ya da ekranın bir bölümünün resmini çekmek için **fotoğraf makinesi** düğmesine tıklayın. Fotoğraf makinesi yalnızca biçimlendirilmiş notlarda bulunur.
- Her şey siz yazarken kendiliğinden kaydedilir. Bir not kaydedilemezse (disk dolu, not klasörü kaybolmuş), durum satırı bunu söyler ve not metniyle birlikte açık kalır: başka bir not, yeni bir not ya da başka bir not klasörü ancak kayıt yapıldıktan sonra açılır ve bir pencere **Metni Kopyala** ve **Yeniden Dene** seçeneklerini sunar. Bir not kaydedilemezken program kapanırsa, metni *~/.local/share/{{APP}}/Emergency* acil durum klasörüne konur (0.65).
- **Projeye Gönder** açık olan her projeyi, başka {{APP}} pencerelerinde açık olanları da listeler. Her biri için: **Araştırma Klasöründe Yeni Belge**, seçtiğiniz bir klasörde **Yeni Belge** ya da bir metin belgesinin **Sonuna Ekle**. Biçimlendirme ve resimler de gelir. Siz silene kadar not karalama defterinde kalır.

Notlar bir klasördeki sıradan dosyalardır; normalde *Documents/{{APP}} Scratch Pad* (düz metin .txt, biçimlendirilmiş notlar .html olarak; resimleri gizli *.images* alt klasöründe). Başka bir klasörü, yeni notların türünü ve karalama defterinin diğer pencerelerin üstünde kalıp kalmayacağını **Dosya > Seçenekler > Genel > Karalama Defteri** altında seçebilirsiniz. Başka yerde (başka bir {{APP}} penceresi, eşitlenen bir klasör) değiştirilen notlar hemen görünür.

**Her programdan.** Programdaki kısayol, klavye {{APP}} üzerindeyken çalışır. Karalama defterini herhangi bir programdan açmak için masaüstünün {{APP}} programını `--scratch-pad` ile başlatmasını sağlayın: KDE'de **Sistem Ayarları > Klavye > Kısayollar > Yeni Ekle > Komut veya Betik** yolunu izleyin, **Dosya > Seçenekler > Klavye** altında gösterilen komutu (**Komutu Kopyala** düğmesi) girin ve istediğiniz tuşlara basın. Karalama defteri bu durumda kendiliğinden, proje penceresi olmadan açılır ve notları açık olan projelere gönderir. {{APP}} uygulama menüsüne eklendiyse (**Yardım > Uygulama Menüsüne Ekle**), menü girdisinde **Karalama Defteri** eylemi de bulunur.

**Fotoğraf makinesi**, kuruluysa KDE'nin ekran görüntüsü programı *Spectacle* ile resmi çeker (programların ekranı kendilerinin okuyamadığı Wayland'de gereklidir): istediğiniz bölümün üzerinde sürükleyin. Spectacle yoksa, X11'de {{APP}} ekranı gösterir ve siz istediğiniz bölümün üzerinde sürüklersiniz; **Esc** iptal eder, **Enter** tüm ekranı alır.

### Konuşma Yazıya Dökme

Yazmak yerine konuşun: **Konuşma Yazıya Dökme** eklentisi (0.61 ya da sonraki sürüm) söylediklerinizi yazıya geçirir. Her şey bu bilgisayarda gerçekleşir; siz konuşurken hiçbir şey hiçbir yere gönderilmez.

**İlk kayıttan önce: bir model seçin.** Konuşma tanıma bir modele ihtiyaç duyar; model bir kez indirilir. **Dosya > Seçenekler > Konuşma** bölümünü (ya da mikrofon düğmesinin yanındaki oku, **Konuşma Yazıya Dökme Seçenekleri…**) açın, bir model seçin ve **İndir ve Kullan** düğmesine basın:

- **Small** (Küçük): 190 MB'lık indirme, hızlı; net konuşma için iyi, çeviri yalnızca kaba.
- **Medium** (Orta, önerilen): 539 MB; son yılların çoğu işlemcisinde konuşmaya yetişir; iyi tanıma ve çeviri.
- **Large v3**: yaklaşık 1,1 GB; en iyisi, ama çoğu işlemcide konuşmadan yavaştır; bu yüzden uzun konuşmalarda metin geride kalır. Bir ekran kartı yardımcı olur.
- **Large v3 Turbo**: 574 MB; Large v3'e yakın ve daha hızlı, ama çeviri yapamaz.

Modeller, whisper.cpp'nin Hugging Face üzerindeki model dosyalarından gelir. **Kaynak** altında bunun yerine başka bir adres verebilir ya da kendiniz indirdiğiniz bir model dosyası için **İndirdiğim Bir Dosyayı Kullan…** düğmesini kullanabilirsiniz. Bir model kullanılmadan önce denetlenir: önce sağlama toplamı (SHA-256), sonra dosyanın içindeki model açıklaması ve kısa bir deneme yazıya dökümü. Her şey yolundaysa kullanımdaki modelin yerini alır; eski model silinir. Resmi kaynaktan gelen bir modelin bilinen sağlama toplamına sahip olması gerekir; aksi halde kurulmaz (indirme bozulmuş olabilir). Başka bir adresten gelen ya da kendinizin indirdiği, sağlama toplamı bilinenle aynı olmayan bir dosya, ancak siz onay verdikten sonra okunur; çünkü konuşma motorunun onu okuması gerekir. Diğer adresler *https://* ile başlamalıdır. Yarıda kalan bir indirme, bir sonraki seferde kaldığı yerden devam eder. **Hız Testi**, bu bilgisayarın seçili model ve aygıtla kısa bir kaydı ne kadar sürede işlediğini gösterir.

**Kayıt.** Araçlar araç çubuğundaki mikrofon düğmesine (ya da **Düzenle > Yazım Araçları > Konuşma Yazıya Dökme**) basın. Denetçi **Yazıya Dökümler** sekmesinde açılır (mikrofon, denetçinin düğme sırasında ilktir) ve tarih ile saatle adlandırılmış yeni bir yazıya döküm başlar. Konuşun; kısa bir duraklama bir parçayı bitirir ve metni birkaç saniye sonra görünür. Daha uzun bir duraklama (siz değiştirmedikçe 3 sn) yeni bir paragraf başlatır. Durdurmak için düğmeye yeniden basın: bekleyen parçalar yazılır, sonra metin düzenlenebilir. Düğmenin yanındaki ok mikrofonu seçer.

**Yazıya dökümler**, her projenin dışında, kendilerine ait bir klasörde (normalde *Documents/{{APP}} Transcriptions*) düz metin dosyaları olarak tutulur; böylece projelerinize dokunulmaz. Sekmede bir yazıya dökümü düzenleyebilir (kendiliğinden kaydedilir), yeniden adlandırabilir, tüm metnini istediğiniz yere yapıştırmak için kopyalayabilir, silebilir (masaüstünün çöp kutusuna gider) ya da klasörünü açabilirsiniz. Bir kayıt yazılırken metni salt okunurdur. Düzenlenen bir yazıya döküm kaydedilemezse açık kalır (durum satırı nedenini söyler); bu sırada bir kayıt başlarsa ya da program kapanırsa, metni *~/.local/share/{{APP}}/Emergency* acil durum klasörüne konur (0.65).

**Şimdi kaydet, metni sonra yaz** (0.64). Metni yazmak çok fazla işlem gücü ister; pille çalışan bir dizüstü bilgisayarda (ya da telefonda) yalnızca kayıt yapıp metnin daha sonra yazılmasını sağlayabilirsiniz. **Dosya > Seçenekler > Konuşma** altında **Yalnızca kaydet** seçeneğini seçin (*Mikrofon başladığında*; Android'de bu varsayılandır). Mikrofon düğmesi bu durumda yalnızca kayıt yapar: başka hiçbir şey çalışmaz, hiçbir model yüklenmez. Kayıt, yazıya dökümlerin yanında, aynı türde bir adla bir ses dosyası olarak (WAV, dakikada yaklaşık 1,9 MB) saklanır ve sekmenin listesinde satırında bir **Yazıya Dök** düğmesiyle görünür; **Tümünü Yazıya Dök** henüz metni olmayan her kaydın metnini sırayla yazar. Metin bundan sonra canlı bir kayıttaki gibi görünür ve düzenlenebilir. **Metin yazıldıktan sonra kaydı sil** işaretini koymadıkça, metni yazıldıktan sonra kayıt saklanır; bir yazıya dökümü yeniden adlandırmak ya da silmek kaydını da birlikte götürür. Metni başka bir bilgisayarda yazmak için, eşitleme programınızın iki bilgisayarda aynı tuttuğu bir yazıya döküm klasörü seçin. Metin yazılırken bir kayıt durdurulursa (ya da program kapanırsa), **Yazıya Dök** düğmesi kalır ve yeniden çalıştırılabilir.

**Diller.** **Ana dil** metnin dilidir; **yedek dil** de konuştuğunuz başka bir dildir (ikisi de programın on bir dilinden seçilir: İngilizce, Almanca, İspanyolca, Fransızca, Portekizce, İtalyanca, Rusça, Endonezce, Türkçe, Lehçe ve Felemenkçe). Ana dil İngilizce iken, yedek dilde söylediğiniz şey İngilizce yazılır (whisper yalnızca İngilizceye çevirir). İngilizce cümleler içindeki yedek dil sözcükleri yanlış duyulabilir (bir denemede "Bahnhof", "Bon Huff" oldu); kullandıklarınızı **İpucu sözcükleri** altına yazın, söylediğiniz gibi yazılırlar ("Bahnhof", "Brötchen"). Başka bir ana dille ya da Large v3 Turbo ile konuşma, söylendiği dilde kalır. **Söylenenleri çevirinin yanında tut** çevrilen bir parçanın ardına özgün metni parantez içinde ekler (daha uzun sürer). **İpucu sözcükleri**: kullandığınız isimler ve sözcükler; böylece sizin yazdığınız gibi yazılırlar.

**Diğer seçenekler.** **Hesaplama aygıtı**: işlemci ya da whisper.cpp'nin Vulkan üzerinden kullanabildiği bir ekran kartı (bulunduğunda gösterilir; model için belleği yetersiz kartlar listelenir ama seçilemez). Windows'ta eklenti şimdilik yalnızca işlemciyi kullanır: orada Small ya da Medium en iyi şekilde yetişir. **İşlemci iş parçacıkları**: işlemcinin sahip olduğundan bir eksik, yazmayı akıcı tutar. **Doğruluk**: *Dikkatli* birkaç olası metni tartar ve her parçanın dilini önce bulur: daha iyi, ama daha yavaş. **Mikrofon**, **Hassasiyet** (gürültülü oda için düşük, kısık ses için yüksek), **Bir parçayı bitiren duraklama**, **Şu kadar duraklamadan sonra yeni paragraf** (*asla* bunu kapatır), **klasör** ve **Gelişmiş** altında: sessizlikte ve gürültüde metni dışarıda bırakma (whisper orada bazen "Thank you for watching" gibi bir satır uydurur) ve bir önceki metni sonraki parça için bağlam olarak kullanma.

---

### Çeviri Yardımcısı

0.92.0'dan beri **Çeviri Yardımcısı** eklentisi (sürüm 1.5; adı 1.4'e kadar Translation and Proofreading idi) {{APP}} programının kendi eklentilerinden biridir ve tam sürümün içinde gelir; elinizdeki bir {{APP}} için Sürümler (Releases) sayfasında kendi başına bir pakettir (Bölüm 17). On bir dilden herhangi ikisi arasında çeviri yapar ve İngilizce metinde yazım ve dilbilgisini denetler; hepsi kendi bilgisayarınızda: metin ondan asla çıkmaz. {{APP}} programının kendisinde yapay zeka işlevi yoktur; bu eklenti dil modelleriyle çalışır ve diğerleri gibi Eklenti Yöneticisi'nden kapatılabilir ya da kaldırılabilir.

- **Bu araç yalnızca öneri verir. Çevirileri kesin değildir: her birini denetleyin; bunun için iki dilin de iş görecek düzeyde bilinmesi gerekir.** Aynı cümle eklentinin penceresinde ve açıklamasında da yazar.
- Denetçide kendine ait bir sekmesi ve kendine ait bir penceresi vardır (komutu Düzenle menüsünün Yazım Araçları alt menüsündedir ve bir araç çubuğu düğmesi de vardır). Metin girer, sonuç her değişiklik işaretlenmiş olarak çıkar; siz yazarken canlı olarak; bir sözcüğe tıklamak özgün halini korur ya da başka sözcükler önerir.
- Bir modele ihtiyaç duyar; modeli eklentinin seçeneklerinde bir kez seçip indirirsiniz (birkaç yüz megabayttan birkaç gigabayta kadar; bilgisayar için fazla büyük bir model sunulmaz). Modeller {{APP}} deposunun *models-1*, *models-2*, *models-3* ve *models-4* sürümlerinden alınır, her biri ancak sağlama toplamı bilinen toplam olduğunda kurulur ve bundan sonra eklenti hiçbir bağlantı olmadan çalışır. Modellerden biri, CoEdIT Large, yalnızca ticari olmayan amaçlarla kullanılabilir; eklenti bunu indirmeden önce söyler.
- **Tek başına bir sözcük** (1.3). Tek bir İngilizce sözcük: sonuç sözcüğün olması gereken halidir; altında anlamı gelir (anlamlar eklentiye yerleşik WordNet 3.0 sözlüğünden gelir; sözlükte olmayan bir sözcük için, indirdiğiniz Qwen3 modeli bir anlam yazar ve bunun yanlış olabileceği uyarısı eklenir; böyle bir model yoksa eklenti nelerin indirilebileceğini söyler), yazım önerileri ve benzer anlamlı sözcükler. Tek bir Almanca sözcük: eklenti önce **Almanca sözlükte** arar; bu, eklentinin seçeneklerinde ayrıca indirdiğiniz Wiktionary'den bir özettir (lisansı, CC BY-SA 4.0, indirmeden önce gösterilir): temel biçim, sözcük türleri, anlamlar ve her anlamın İngilizce sözcükleri. Bir türün ilk beş anlamı gösterilir, gerisi "more" (daha fazla) bağlantısının arkasındadır; bir İngilizce sözcüğe tıklamak onu sonuç yapar ve "Similar" (Benzer) bölümü bundan sonra o sözcüğü ve türünü izler. Sözlüğün söylemediği şeyler için ve sözlükte olmayan bir sözcük için küçük model sözcüğü çevirir ve çıktı bunun yanlış olabileceğini uyarır. İngilizceden Almancaya, WordNet'in bildiği sözcük türlerine göre tek bir İngilizce sözcük için aynı şekilde çalışır.
- **Değiştir ve geri çevir** (1.3). **Değiştir** (Swap) Almancadan İngilizceye çeviriyi İngilizceden Almancaya çevirir ve tersini yapar: sonuç metne taşınır ve öbür yönde çevrilir. **Geri çevir** (Translate back) sonucu metninizin diline geri çevirir ve metni değiştirene kadar sonucun altında gösterir; geri dönüş yönü için modele ihtiyaç duyar (Almanca bir metin için OPUS-MT English to German (small)) ve eksikse indirmeyi önerir. Çeviri yaparken denetimlerin ikinci satırı çeviri modelini gösterir (başka birini seçmek ayarı ve çeviriyi değiştirir) ve stil listesi gizlenir.
- **Kısa metinler** (1.3). Tek bir sözcük ve en çok üç sözcüklük bir metin hiçbir zaman büyük çeviri modeline verilmez (model tam cümleler için daha iyidir, tek sözcüklerde güvenilmezdir): önce sözlük, o olmazsa küçük model; büyük model seçilmiş olsa bile. Birkaç sözcüklük kısa bir metin için "Other translations:" (Diğer çeviriler) modelin en iyi tam çevirilerini, uydurduğu isimler olmadan listeler.
- **Sürüm 1.4.** İki yönde de tek bir sözcük artık sonucun altında, sahip olduğu her sözcük türünün çevirilerini gösterir (küçük harfle fiil, büyük harfle isim olan bir Almanca sözcük ikisini de alır; sözlükte olmayan bir sözcük iki biçimde de çevrilir), sonra "Similar translations" (İngilizce sonucun benzer anlamlı sözcükleri) ya da İngilizceden Almancaya "Similar German words" (anlamları İngilizce sözcüğü ya da eş anlamlılarından birini adlandıran Almanca sözcükler; bunun için Almanca sözlük gerekir, yoksa bir satır bunu söyler), sonra kısa açıklama gelir. "Automatic" (Otomatik) ile, İngilizce sözcük olmayan tek bir Almanca sözcük, Almanca sözlük kuruluysa Almanca olarak tanınır. Almanca çevrilmeden önce, Almanca isimlerin büyük harfleri modele giden kopyada düzeltilir (metniniz olduğu gibi kalır): sözlüğün yalnızca isim olarak bildiği bir sözcük büyük harfini alır, iki türlü bilinen bir sözcük ("essen") yazdığınız biçimi korur, ancak bir artikelin ya da artikelli bir edatın hemen ardında bundan ayrılır ("das Essen", "im Essen"); sözlük yoksa çevirinin altındaki bir not Almanca isimlerin büyük harf gerektirdiğini söyler. Yalnızca büyük harfte birbirinden ya da çeviriden ayrılan alternatifler bir kez gösterilir. Sözcük alanı artık sonucun sözcükleri için "Spelling" (Yazım) satırı göstermez (kendi metninizde tıkladığınız sözcükler onu korur) ve benzer sözcüklerin ek kuralları artık "number" sözcüğünü "numb" olarak okumaz. Bir şey eksik olduğunda (bir çeviri modeli, Almanca sözlük), eklentinin sekmesinin ve penceresinin en üstündeki bir bildirim bunu adlandırır ve eklentinin seçeneklerini açan bir düğmesi vardır. 15 dakika kullanılmayan bir model belleği boşaltmak için bellekten kaldırılır (**Dosya > Seçenekler > Düzeltme Okuması > Hız > Modelleri şu süreden sonra bellekten kaldır**; "asla" modeli yüklü tutar). Boş alanlardaki ipuçları dar genişlikte satır kaydırır.
- **Sürüm 1.5: on bir dil.** Eklenti İngilizce, Almanca, İspanyolca, Fransızca, Portekizce (Brezilya), İtalyanca, Rusça, Endonezce, Türkçe, Lehçe ve Felemenkçe dillerinden herhangi ikisi arasında çeviri yapar: **Çevir** seçiliyken iki açılır liste metninizin dilini ve çevirinin dilini adlandırır. Her dilin, İngilizceye ve İngilizceden giden iki küçük OPUS-MT modeli vardır (lisansları, Apache-2.0 ya da CC BY 4.0, bir indirmeden önce seçeneklerde gösterilir); İngilizce içermeyen bir çift (örneğin İspanyolcadan Rusçaya) iki modelle İngilizce üzerinden gider ve sonucun altındaki bir not bunu söyler, çünkü ilk adımın bir hatası ikinciye taşınır. İngilizce dışındaki her dilin ayrıca Wiktionary'den isteğe bağlı bir **sözlüğü** vardır (CC BY-SA 4.0, lisansı önce gösterilen ayrı bir indirme); o dilin tek bir sözcüğü için, Almanca sözlüğün kullanıldığı gibi kullanılır. Kullanılabilir sözlüğü olmayan bir dil yalnızca modelin çevirilerini alır. Tek bir sözcük yine bir öneri olarak düzeltilir: yazım satırı ve altındaki sözcükler bunların denetlenmesi gereken öneriler olduğunu söyler. Eklentinin kendi metinleri programın on bir dilinde gelir ({{APP}} için seçtiğiniz dil; bir metin çevrilmemişse İngilizce). 1.4'ün ayarları çalışmaya devam eder: Almanca sözlük ve indirdiğiniz modeller kalır ve ilk seçim Almancadan İngilizceye çeviridir. Seçenekleri artık **Dosya > Seçenekler > Translation Helper** altındadır.
- 1.1 sürümüyle indirdiğiniz modeller olduğu gibi kalır. 1.1 sürümü modelleri eklentinin eski deposundan alıyordu; 1.2 yeni yeri bilir.
- Eklentinin kendi açıklaması, düzeltme okuması kurallarıyla (Amerikan yazımı, kıvrık tırnaklar, uzun tire yok) ve her modelin neyde iyi olduğuyla birlikte, {{APP}} kaynağındaki *addons/translation-proofreading/source/README.md* dosyasındadır.

## 11. Anlık görüntüler

Anlık görüntü (snapshot), bir belgenin belirli bir andaki kayıtlı kopyasıdır. Büyük bir yeniden yazımdan önce bir tane alın:

- En az bir anlık görüntüsü olan bir belge, Binder'daki simgesinin sağ üstünde katlanmış bir köşe (açık gri) gösterir.
- **Ctrl+5** bir anlık görüntü alır; **Ctrl+Shift+5** bir başlık sorar.
- **Dosya > Seçenekler > Genel > Kaydetme** ile **Dosya > Kaydet** (**Ctrl+S**), son Dosya > Kaydet işleminden bu yana değiştirdiğiniz her metnin bir anlık görüntüsünü alır ("Saved" başlığıyla); **Dosya > Seçenekler > Davranışlar > Anlık Görüntüler**, anlık görüntüler alındığında durum çubuğunda kısa bir ileti gösterir.
- Denetçinin anlık görüntü sekmesinde bir anlık görüntüyü geçerli metinle **karşılaştırabilir** (eklemeler renkle, silmeler üstü çizili gösterilir; sözcükleri, cümleleri, paragrafları ya da karakterleri karşılaştırmayı seçebilirsiniz) ve ona **geri dönebilirsiniz**. Geri dönmeden önce {{APP}} geçerli metnin bir anlık görüntüsünü alabilir; böylece hiçbir şey kaybolmaz. Anlık görüntü önce okunur (0.65): bütünüyle okunamayan bir anlık görüntü geçerli metni olduğu gibi bırakır, hiç metin içermeyen bir anlık görüntü onun yerini ancak ikinci bir sorudan sonra alır ve geçerli metnin anlık görüntüsü istenmiş ama alınamamışsa hiçbir şeye geri dönülmez. Geri dönmek, dosyası bozuk ya da başka bir karakter kodlamasında olan bir belgeyi (salt okunur gösterilir) onarmanın da yoludur: bozuk dosya önce *settings/conflicts* içine kopya olarak saklanır ve denetçi nerede olduğunu söyler (0.81.1).
- **Proje > Eski Anlık Görüntüleri Sil…** (0.61.1'den beri, Çöp Kutusunu Boşalt'ın altında) projeyi yeniden küçültmek için projenin her anlık görüntüsünü, Çöp Kutusu'ndaki belgelerinkiler dahil, siler. Metinler şimdiki halleriyle kalır; yalnızca önceki sürümleri gider. Önce bir soru, kaç anlık görüntü olduğunu, kaç belgeye ait olduklarını ve ne kadar yer kapladıklarını söyler; bu geri alınamaz, bu yüzden önce bir yedek alın (**Dosya > Yedekle**). Projede anlık görüntü yoksa komut kapalıdır. Scrivener biçimindeki bir projede anlık görüntüler *.scriv* içinden de çıkar (hemen ardından gelen kayıtla); 0.61.1'den beri bu, denetçide sildiğiniz tek bir anlık görüntü için de böyledir.

---

## 12. Yazım modu

**F11**, ekranı bir kâğıt sayfasındaki metninizden başka hiçbir şeyle kalmayacak şekilde doldurur. Denetim çubuğunu göstermek için fareyi alt kenara götürün: yakınlaştırma, kâğıt genişliği, arka planın solması, daktilo kaydırma, sözcük sayısı ve özet ile notların bulunduğu küçük bir denetçi. **Alt+Shift+Up / Down** önceki ya da sonraki belgeye geçer, **Ctrl+[** ve **Ctrl+]** baktığınız belgelerde geri ve ileri gider. Biçim ve Ekle menülerinin kısayolları (kalın, stiller, yorumlar ...) burada da çalışır. **Esc** ya da **F11** ana pencereye döndürür.

**Dosya > Seçenekler > Davranışlar > Yazım Modu**, hangi ekranda açılacağını (geçerli, birincil ya da adıyla seçilen bir ekran), ana pencerenin yoldan çekilip çekilmeyeceğini (yazım modu açıkken simge durumuna küçültülür) ve **Esc** tuşunun onu kapatıp kapatmayacağını (**F11** her zaman kapatır) seçer.

Denetim çubuğu her temada koyudur; bu nedenle düğmeleri, yakınlaştırma listesi ve metinleri açık temalarda da okunabilir kalır (0.94.0). Metnin, kâğıdın ve arka planın renkleri temadan gelir. Bunları değiştirmek için, örneğin beyaz üzerine siyah ya da siyah üzerine yeşil için, **Dosya > Seçenekler > Dış Görünüm > Yazım Modu** bölümüne gidin; değişiklik yazım modu açıkken bile hemen görünür. Bir arka plan resmi, projeye özel olarak **Proje Ayarları > Özel Klasörler** altında belirlenebilir.

---

## 13. Derleme: el yazmasını oluşturmak

**Dosya > Derle…** (**Ctrl+Shift+E**), Taslaktaki her şeyi tek bir dosyada bir araya getirir.

### Derleme penceresi

Derleme penceresinde en üstte **Şunun İçin Derle**, solda biçimler, ortada kesim düzenleri, sağda içerik, üst veri, seçenekler ve değiştirme kuralları bulunur (simgeli dört sekme; adını görmek için fareyi sekmenin üzerinde tutun). Biçimler sütunu en az en uzun adı kadar geniştir; üç sütun arasındaki çizgileri sürükleyerek birini genişletebilirsiniz; pencere genişlikleri bir sonraki sefer için saklar.

1. **Şunun İçin Derle**: çıktı biçimi. Başka bir programa gerek kalmadan yerleşik olarak: **Word Belgesi (.docx)**, **PDF (.pdf)**, **E-kitap (.epub)** ve senaryolar için **Fountain** ile **Final Draft**. LibreOffice üzerinden PDF ve OpenDocument, Pandoc üzerinden EPUB ve Markdown da, ilgili eklenti ve program kuruluysa görünür (aşağıya bakın). Yanında: kaç belgenin ve yaklaşık kaç sözcüğün derleneceği.
2. **Biçimler** (solda), **Biçimlerim** ve **Yerleşik Biçimler** altında: derleme biçimi el yazmasının nasıl görüneceğini belirler: yazı tipleri, satır aralığı, sayfa boyutu, kenar boşlukları, üst ve alt bilgiler, bölümlerin ve sahnelerin görünümü, stillerin, notların ve alıntıların nasıl çıkacağı. Yerleşik biçimler (*El Yazması (Times)*, *El Yazması (Courier)*, *Modern*, *Ciltsiz Kitap (6 x 9 inç)*, *E-kitap*, *Varsayılan (Düzenleyici Biçimlendirmesi)* ve *Anahat (Özetler)*) değiştirilemez. Listenin altındaki düğmeler: **+** (seçili biçimi çoğaltarak kendi sürümünüzü yapın ve düzenleyin), kalem (kendi biçiminizi düzenleyin), **-** (silin), **İçe Aktar...** ve **Dışa Aktar...** (başka bir bilgisayarda kullanmak ya da vermek için bir *.qbformat* dosyası; adı zaten kullanılan içe aktarılmış bir biçim bir numara alır).
3. **Kesim Düzenleri** (ortada): **Yazı Tipi** tüm el yazması için tek bir yazı tipi belirler ya da bunu düzenlere bırakır ("Kesim Düzenine Göre Belirlenir"). Tablo, projenizdeki her kesim türünü, bu biçimde alacağı düzenle birlikte listeler; listeden başka birini seçin. Tablonun altında küçük bir resim, seçili düzene sahip bir kesimin nasıl başladığını gösterir. **Kesim Düzenlerini Ata...** solda kesim türlerinin, sağda biçimin tüm düzenlerinin resimlerinin bulunduğu daha büyük bir pencere açar. Yerleşik biçimler kesim türlerinin olağan adlarını bilir: örneğin *El Yazması (Courier)* biçiminde *Kısım* **Kısım Başlığı**, *Bölüm* **Bölüm Başlığı**, *Sahne* ya da *Kesim* ise **Kesim Metni** alır.
4. **İçerik** (ilk sekme): nelerin derlendiği, her belgenin kesim türü ve düzeniyle. Taslağın yalnızca bir bölümünü (örneğin tek bir bölümü) derlemek için bir klasör seçin, ön sayfalar (başlık sayfası) ve son sayfalar (teşekkürler) ekleyin. **Yazarın adı** (0.94.0): proje şablonlarının başlık sayfası "yazan" ve yazarın adını içerir. Başlık sayfası yazan ilk derleme bu adı sorar (ad ve soyad; **Dosya > Seçenekler > Genel** altındaki Yazar Bilgileri ile birlikte saklanır, bu yüzden bir kez sorulur); boş bırakırsanız "yazan" satırı her biçimde başlık sayfasından çıkarılır. Derleme penceresinin Üst Veri sekmesindeki Yazar alanında bir ad varsa, derlenen dosya için bunun yerine o kullanılır. Bir belgeyi dışarıda bırakmak için işaretini kaldırın; bu, **Derlemeye Dahil Et** ayarını kapatır. **Nesne sayfaları ekle**, Nesneler sekmesindeki (Bölüm 7) her nesne için son sayfalardan sonra bir sayfa koyar: her kategori için bir başlık (Karakterler, Mekânlar ...), sonra her nesne alanlarıyla birlikte; yanındaki düğme kategorileri seçer. Başlangıçta kapalıdır.
5. **Üst Veri** (ikinci sekme): başlık, alt başlık, yazar, dil (örneğin *en-US*), konu, anahtar sözcükler, yayıncı, tarih, açıklama, e-kitabın kapağı (Binder'dan bir resim) ve kitap tanımlayıcısı. Boş alanlar proje başlığını, adınızı ve bugünün tarihini alır. Ayrıntılar PDF'in belge özelliklerine, e-kitaba ve Word dosyasına girer; `<$projecttitle>` gibi yer tutucular burada da çalışır.
6. **Seçenekler** (üçüncü sekme): klasörlerin metninin derlenip derlenmeyeceği.
7. **Değiştirme Kuralları** (dördüncü sekme): bu projenin, biçimin değiştirmelerinden sonra yapılan değiştirmeleri; örneğin taslaklarınızda bir şekilde, kitapta başka şekilde yazılan bir ad. Her satır açılıp kapatılabilir ve düzenli ifade, büyük/küçük harf duyarlı ya da yalnızca tam sözcük olabilir.
8. **Önizleme**, el yazmasının sayfalarını, seçtiğiniz biçim ve içerikle, hiçbir şey kaydedilmeden önce, ayrı bir pencerede PDF olarak gösterir.

Yazı tipi, değiştirme kuralları ve nesne sayfaları, diğer seçimler gibi projeyle birlikte saklanır ({{APP}} programının eski sürümleri bunları yok sayar).

**Derle…** düğmesine tıklayın, nereye kaydedeceğinizi seçin; isterseniz bitmiş dosya metin işlemcinizde ya da okuyucunuzda açılır. **Ayarları Kaydet**, seçimleri derlemeden saklar.

**El ile ayarlanan yazı tipleri ve paragraf biçimlendirmesi.** *Varsayılan (Düzenleyici Biçimlendirmesi)* biçimiyle, derlenen dosya hepsini korur: yazı tipleri, boyutlar, renkler, büyük harfler, harf aralığı, girintiler, aralıklar, satır aralığı ve sekme durakları. Diğer her biçimin kendine ait bir görünümü vardır: yazı tipini, boyutu, girintileri ve aralıkları değiştirir; metin rengini, büyük harfleri, harf aralığını, hizalamayı ve sekme duraklarını korur.

### Bir derleme biçimini düzenlemek

**Çoğalt ve Düzenle…** ile **Düzenle…** biçim düzenleyicisini açar. Soldaki liste sayfalarını içerir:

- **Metin**: yazı tipi, boyut, satır aralığı, ilk satır girintisi, paragraflardan sonraki boşluk, iki yana yaslanmış metin, başlıkların yazı tipi ve boyutu, kalın başlıklar. **Projenin düzenleyici biçimlendirmesini kullan**, bunun yerine düzenleyicinin görünümünü korur.
- **Kesim Düzenleri**: biçimin düzenleri (ekleyin, çoğaltın, kaldırın); başlık, ön ek ve son ek (`Chapter <$W:chapter>`), başlık stili ve hizalama, büyük harf, başlıktan önceki, metinden önceki, özetten önceki ve notlardan önceki boş satırlar, yeni sayfa, kesimden önceki ayırıcı, ilk paragrafta girinti olmaması. Önizleme bu düzene sahip bir kesimin nasıl başladığını gösterir.
- **Atamalar**: bu biçimde her kesim türünün hangi düzeni alacağı.
- **Stiller**: projenizin bir stilinin bu biçimde nasıl çıkacağı. **Stil ekle…**, sonra seçin: **Görünümünü koru**, **Görünümünü değiştir** (yazı tipi, boyut, kalın, italik, altı çizili, küçük ya da tüm büyük harfler, renk, hizalama, girintiler, öncesinde ve sonrasında boşluk, satır aralığı, öncesinde ve sonrasında metin, büyük harf, yeni sayfada başla; yarım işaretli ya da "değişmedi" olan bir kutu stili olduğu gibi bırakır), **Gövde metni yap** ya da **Metni dışarıda bırak** (örneğin kendi stiline sahip kendinize notlar).
- **Sayfa**: kâğıt boyutu ve kenar boşlukları, kitaplar için yansıtılmış kenar boşlukları (iç ve dış), üst ve alt bilgiler (sol, orta, sağ; sayfa numarası için `<$p>` ve `<$title>` gibi yer tutucular), karşılıklı sayfalar (sol metinler sol sayfalarda, sağ metinler sağ sayfalarda), ilk sayfada ve bir bölümün başladığı sayfalarda üst ve alt bilgi, bölümlerin sağ sayfada başlaması, ilk sayfa numarası ve stili (1, 2, 3 / i, ii, iii / I, II, III), üst ve alt bilgilerin boyutu.
- **Dipnotlar ve Yorumlar**: sayfanın altında dipnotlar (e-kitaplarda: açılır notlar), her bölümün sonunda ya da metnin sonunda (PDF ve e-kitaplar; Word dosyasında dipnotlar her zaman sayfanın altındadır), notların başlığı ve boyutuyla; satır içi açıklamalar yorum olarak ya da kaldırılmış; bağlantılı yorumlar Word dosyasının yorumları olarak; vurgular kaldırılmış.
- **Değiştirme Kuralları**: yalnızca derlenen dosyada değiştirilen metin (projeniz olduğu gibi kalır): açık ya da kapalı, büyük/küçük harf, tam sözcükler, düzenli ifadeler ("Şununla" içinde `\1` ile `\9` arası).
- **Dönüşümler**: düz ya da tipografik tırnaklar, tireler kısa çizgi olarak, italikler altı çizili olarak (el yazması geleneği), birden çok boşluk tek boşluk olarak, paragraf sonlarında boşluk yok, kalın, italik, yazı tipi ya da renk içermeyen düz metin.
- **İçindekiler, PDF ve E-kitap**: bir içindekiler tablosu (başlığı ve kaç başlık düzeyi), başlıklar için PDF yer imleri, PDF'te tıklanabilir web bağlantıları, e-kitapta yeni bir sayfada başlayan her kesim için bir dosya ve içindekiler tablosu e-kitabın bir sayfası olarak da.

Biçimleriniz *~/.local/share/{{APP}}/CompileFormats* içinde saklanır ve her projede kullanılabilir.

### PDF

Yerleşik PDF çıktısı sayfaları kendisi düzenler: dipnotlar, kendilerine atıf yapılan sayfanın altında (uzun bir dipnot sonraki sayfaya devam eder), üst ve alt bilgiler, biçim öyle diyorsa bölümler sağ sayfalarda, başlıklar kendilerini izleyen metinle birlikte tutulur, sayfanın üstünde ya da altında tek başına kalan satır olmaz, tıklanabilen sayfa numaralı bir içindekiler tablosu, okuyucunun yan panelinde başlıklar için yer imleri ve tıklanabilir bağlantılar. Yazı tipleri gömülüdür. Resimler metindeki boyutunu korur; metin alanından büyük bir resim sığması için küçültülür. Metnin yanındaki resimlerin ve tabloların çevresinden, düzenleyicide olduğu gibi, metin akar. Bir tablo gerektiği kadar sayfaya, satır satır ve başlık satırları her sayfanın üstünde olacak şekilde devam eder.

### E-kitaplar

Yerleşik e-kitap çıktısı EPUB 3 yazar (eski okuyucular için de bir içindekiler tablosuyla): her bölüm için bir dosya, okuyucunun içindekiler tablosunda başlıklar, açılır notlar olarak dipnotlar (ya da her bölümün ya da kitabın sonunda), resimler ve **Üst Veri…** bölümünden kapak. Tablolar tasarımlarını korur; altyazılı bir resim bir şekildir (düzenleyicide öyleyse metnin yanında) ve açıklaması resmin ekran okuyucuların okuduğu alternatif metnidir. Kitap, ilk derlendiğinde bir tanımlayıcı alır; bu proje ile birlikte kalır, böylece bir okuyucu yeni bir sürümü aynı kitap sayar. Okuyucular kendi yazı tiplerini ve sayfa boyutunu seçer; bu yüzden sayfa ayarları, üst ve alt bilgiler geçerli değildir.

### Word

Word dosyası adlandırılmış stiller kullanır (Başlık, Alt Başlık, Başlık 1 ile 3, Alıntı, Ortalanmış, Resim Yazısı ve kendi paragraf ve karakter stilleriniz); böylece başlıkları Word'ün gezinti bölmesinde görünür. **İçindekiler tablosu ekle** ile başa bir içindekiler tablosu alanı konur ve dosya, metin işlemcisinden açıldığında alanlarını güncellemesini ister; bu sayfa numaralarını doldurur; bir metin işlemcisi bunu yapmazsa alanı orada kendiniz güncelleyin. Karşılıklı üst bilgiler, ilk sayfa ve ilk sayfa numarası biçimin **Sayfa** ayarlarından gelir. Tablolar çizgileri, renkleri, birleştirilmiş hücreleri ve başlık satırlarıyla (Word'de her sayfada tekrarlanır) Word tablolarıdır. Metnin yanındaki bir resim, resim yazısıyla birlikte çizgisiz tek hücreli bir tabloya konur; böylece Word ve LibreOffice metnin ikisinin çevresinden de akmasına izin verir; resmin açıklaması alternatif metnidir.

### Başlıklar ve yer tutucular

Bir kesim düzeninde başlık, Derle'nin değiştirdiği yer tutucular içerebilir:

| Yer tutucu | Sonuç |
|-------------|--------|
| `<$n>` | 1, 2, 3 … |
| `<$w>` / `<$W>` / `<$WW>` | one, two … / One, Two … / ONE, TWO … |
| `<$R>` / `<$r>` | I, II, III … / i, ii, iii … |
| `<$L>` / `<$l>` | A, B, C … / a, b, c … |
| `<$n:name>` | *name* adlı ayrı bir sayaç (yukarıdaki her sayı biçimiyle çalışır) |
| `<$title>` | belge başlığı |
| `<$projecttitle>` / `<$shorttitle>` | proje başlığı / ilk üç sözcüğünün büyük harfle yazılmışı |
| `<$author>`, `<$forename>`, `<$surname>` | **Dosya > Seçenekler** ya da proje ayarlarındaki adınız |
| `<$wc>` / `<$wc100>` / `<$wc1000>` | sözcük sayısı, tam ya da yuvarlanmış |
| `<$date>` / `<$year>` | bugünün tarihi (yyyy-aa-gg) / yıl |
| `<$p>` | sayfa numarası (yalnızca üst ve alt bilgilerde) |
| `<$pages>` | sayfa sayısı (PDF dosyalarının ve çıktıların üst ve alt bilgilerinde) |

Örnek: `Chapter <$W:chapter>` başlığı *Chapter One*, *Chapter Two* ve bu şekilde devam eder.

Yer tutucular metnin içine de yazılabilir (**Ekle > Otomatik Numara** ve **Ekle > Yer Tutucular**). **Yardım > Tüm Yer Tutucuların Listesi…** hepsini listeler; kopyalamak için satırları seçip **Ctrl+C** tuşlarına basın.

### Derleme sırasında stiller

Başlıklar, Başlık, Alt Başlık, Blok Alıntı, Ortalanmış ve Resim Yazısı görünümlerini derleme biçiminden alır. Diğer her stil, her derleme biçiminde kendi biçimlendirmesini (girintiler, aralıklar, hizalama, yazı tipi, renk) getirir. **Varsayılan (Düzenleyici Biçimlendirmesi)** ile yerleşik stillerde yaptığınız değişiklikler de aktarılır. Bir biçimin **Stiller** sayfası bunu her stil için değiştirir (bkz. *Bir derleme biçimini düzenlemek*).

### Derleme sırasında notlar

Bir derleme biçiminin **Dipnotlar ve Yorumlar** sayfasında, bir PDF ya da e-kitapta dipnotların nereye gideceğini (sayfanın altı, her bölümün sonu, metnin sonu), satır içi açıklamaların Word dosyasında yorum olup olmayacağını ya da kaldırılıp kaldırılmayacağını, bağlantılı yorumların korunup korunmayacağını ve vurguların kaldırılıp kaldırılmayacağını seçersiniz. Bir Word dosyasında dipnotlar her zaman sayfanın altında gerçek dipnotlardır.

### Daha fazla çıktı biçimi (uzantılar)

{{APP}} bu biçimlerle de gelir. Her biri ayrıca kurulan bir program gerektirir; LibreOffice ve Pandoc üzerinden PDF ve EPUB, yerleşik olanlara bir alternatiftir (Word dosyasından yola çıkar ve ona benzer):

| Biçim | Gerektirdiği |
|--------|-------|
| PDF (LibreOffice üzerinden) | LibreOffice (çoğu Ubuntu ve Kubuntu sisteminde kuruludur) |
| OpenDocument Metin (.odt, LibreOffice üzerinden) | LibreOffice |
| E-kitap (.epub, Pandoc üzerinden) | Pandoc (Discover'dan "pandoc" kurun) |
| Markdown (.md) | hiçbir şey |

**Dosya > Seçenekler > Uzantılar** hangi biçimlerin hazır olduğunu ve hangilerinde bir program eksik olduğunu gösterir. Daha fazla biçim eklenti olarak kurulabilir (Bölüm 17).

### Senaryoları derlemek

Senaryolar için, başka bir programa gerek duymayan iki çıktı üretilir:

- **Fountain Senaryosu (.fountain)**: Highland, Fade In, Beat, Trelby ve diğer senaryo yazım programlarının okuduğu düz bir metin dosyası.
- **Final Draft (.fdx)**: Final Draft'ın dosya biçimi; çoğu senaryo yazım programı bunu da açabilir.

İkisinde de her paragraf ne olduğunu korur (sahne başlığı, eylem, karakter, parantez içi yönerge, diyalog, geçiş, çekim); senaryoyu öbür program kendi yöntemiyle düzenler. Derleme biçiminin eklediği klasör ve belge başlıkları bölümler (Fountain) ya da perde başlıkları (Final Draft) olur; senaryo öğesi olmayan metin eylem olur. Proje başlığı ve adınız (Dosya > Seçenekler) başlık sayfasını oluşturur. Satır içi açıklamalar Fountain dosyasında not olur ve Final Draft dosyasında dışarıda bırakılır.

Word ya da PDF için **Varsayılan (Düzenleyici Biçimlendirmesi)** derleme biçimini seçin: öğelerin girintilerini, büyük harflerini ve yazı tipini düzenleyicide gördüğünüz gibi korur. Diğer derleme biçimleri yazı tipini ve boyutu değiştirir ama girintileri korur.

---

## 14. İçe ve dışa aktarma

### Dosyaları içe aktarmak

**Dosya > İçe Aktar > Dosyalar…** (**Ctrl+Shift+I**) metin dosyalarını (.txt, .md, .rtf, .docx, .odt; eski Word dosyaları .doc aşağıdaki eklentiyle), senaryoları (.fountain, .fdx), resimleri, PDF'leri, ses ve videoyu ve diğer dosyaları Binder'a ekler. Dosyaları dosya yöneticisinden Binder'a sürükleyebilirsiniz de. Metin dosyaları düzenlenebilir belgeler olur; resimler ve PDF'ler düzenleyicide gösterilir; ses ve video, Medya Oynatma eklentisiyle (Bölüm 6) orada oynatılır. Projeye kopyalanamayan bir dosya (disk dolu, okunamayan bir dosya) bir iletiyle adlandırılır ve eklenmez; böylece geride boş bir öğe kalmaz (0.92.0).

Dosyaların nasıl okunacağı **Dosya > Seçenekler > Paylaşım > İçe Aktar** altında belirlenir: Word, LibreOffice ve RTF dosyalarının yorumlarının satır içi açıklama olup olmayacağı (aksi halde metnin yanında yorum olarak kalırlar), dipnotların satır içi dipnot (varsayılan) ya da bağlantılı dipnot olup olmayacağı, dosyaların paragraf ve karakter stillerinin dışarıda bırakılıp bırakılmayacağı (metin projenizin görünümünü alır; kalın, italik, notlar ve bağlantılar kalır), düz metin dosyalarının kodlaması (otomatik: UTF-8, değilse Windows Batı) ve *.html* dosyalarının web sayfası belgeleri yerine metin belgeleri olup olmayacağı.

Dosyaların nereye gideceği: içine aktarmak için Binder'da bir klasör seçin (dosyalar sona gelir) ya da hemen altına koymak için bir belge seçin. Taslak yalnızca metin alır; oraya bırakılan diğer dosyalar Araştırma'ya gider.

**İçe Aktar ve Böl…** uzun bir dosyayı okur ve çok sayıda belgeye keser; örneğin yalnızca `#` içeren her satırda ya da her çift boş satırda. 0.94.0'dan beri bir .docx, .odt ya da .md dosyasını **başlıklarından** da kesebilir: Başlık 1 bir klasör olur, Başlık 2 bir klasör ya da bir belge olur (pencerede siz seçersiniz), başlıklar arasındaki metin belgeler olur ve daha alt başlıklar metnin içinde kalır. Pencere, içe aktarmadan önce ortaya çıkacak Binder'ı bir ağaç olarak gösterir. Hiçbir başlığın adlandırmadığı bir belgenin başlığı, pencerede yazılı tek bir kuralı izler: metninin boş olmayan ilk satırı (uzunsa 120 karaktere kısaltılır) ya da **Her kesimin ilk satırını başlığı olarak kullan** kapalıysa "<dosya adı> <numara>".

### Eski Word dosyaları (.doc)

Word 97 ile 2003 belgeleri *.doc* (şablonlar *.dot*) olarak kaydediyordu. **Word 97–2003 İçe Aktarma** eklentisiyle (Bölüm 17) metin belgeleri olurlar; eklenti yoksa bir .doc dosyası olduğu gibi Binder'a eklenir ve hangi eklentinin onu okuduğu size söylenir. Eklenti başka bir programa gerek duymaz. Şunları korur:

- metin; Yunanca, Kiril, Asya yazıları ve benzerleri dahil;
- başlıklar (Başlık 1 ile 6), başlık, alt başlık, alıntılar ve resim yazıları; bunlar projenizin stillerini alır; projenizde de bulunan (aynı adlı) bir belge paragraf stili o stili alır;
- kalın, italik, altı çizili, üstü çizili, küçük ve tüm büyük harfler, üst simge ve alt simge, vurgulama; ortalanmış, sağa hizalı ve iki yana yaslı paragraflar;
- düzeyleriyle madde işaretli ve numaralı listeler (numaraları {{APP}} üretir; bu yüzden başlıkların "Chapter 1" numaralandırması devralınmaz);
- tablolar, her satır için bir satır olarak ve hücreler sekmelerle ayrılmış;
- dipnotlar ve son notlar (dipnot olarak), yorumlanan sözcüklerle birlikte yorumlar (yazarın adı yorumda başta gelir);
- bağlantılar, alanların sonucu (tarihler, sayfa numaraları en son gösterildikleri gibi), sayfa ve kesim sonları, satır sonları;
- metindeki resimler (JPEG, PNG, BMP, TIFF);
- RTF projelerinde ayrıca el ile ayarlanan yazı tipleri, boyutlar, renkler ve girintiler (Markdown projeleri, her içe aktarmada olduğu gibi projenin görünümünü korur).

Dışarıda bırakılanlar: üst ve alt bilgiler, kayan resimler ve çizimler, eski metafile biçimlerindeki (WMF, EMF) resimler, gömülü nesneler, gizli metin ve değişiklik izleme ile silinen metin (eklenen metin kalır). Metin kutularının metni belgenin sonuna eklenir. İçe aktarmadan sonra bir ileti nelerin dışarıda bırakıldığını söyler.

**Dosya > Seçenekler > Paylaşım > Dönüştürme**, *.doc* dosyaları için LibreOffice'i ilk tercih yapabilir (üst bilgileri, çizimleri ve Word 6/95 dosyalarını da okur); seçilen yol başarısız olursa diğeri denenir. Word 6.0 ve Word 95 dosyaları ile eklentinin okuyamadığı dosyalar, kuruluysa LibreOffice ile dönüştürülür (Ubuntu'da: `sudo apt install libreoffice-writer`); aksi halde ileti bunu söyler. Parola korumalı dosyalar önce parolasız olarak açılıp kaydedilmelidir. Bazı programların kaydettiği gibi aslında bir RTF dosyası, bir Word 2007 dosyası (.docx) ya da bir web sayfası olan bir *.doc* dosyası, eklenti olmadan da gerçekte ne ise o olarak okunur.

### Web sayfalarını içe aktarmak

**Dosya > İçe Aktar > Web Sayfası…** web'den bir sayfayı projenize getirir. İşi **Web Sayfası İçe Aktarma** eklentisi (Bölüm 17) yapar; eklenti yoksa komut bunu söyler.

1. Sayfayı alacak klasörü seçin (aksi halde **Araştırma**'ya gider).
2. **Dosya > İçe Aktar > Web Sayfası…** komutunu seçin. Pano bir web adresi tutuyorsa zaten doldurulmuştur; aksi halde birini yazın ya da yapıştırın.
3. **Yükle** düğmesine tıklayın (ya da Enter'a basın). Sayfa ve resimleri getirilir; pencere, sözcük ve resim sayısı, yazar, site ve tarihle birlikte nelerin içe aktarılacağını gösterir.
4. İsterseniz **Başlık** alanını düzeltin, aşağıdaki seçimleri denetleyin ve **İçe Aktar** düğmesine tıklayın.

**Yalnızca makale** istediğiniz metni korur; menüleri, reklamları, çerez bildirimlerini, paylaşım düğmelerini, yorum bölümlerini ve diğer makalelerin listelerini dışarıda bırakır. {{APP}} makaleyi okuma araçlarının yaptığı gibi bulur (en çok cümlesi, virgülü ve en az bağlantısı olan metin kazanır; sayfadaki "article" ya da "sidebar" gibi adlar bir bölümün lehine ya da aleyhine sayılır). Bir şey eksikse, tüm sayfayı almak için işaretini kaldırın. **Resimler** makalenin resimlerini getirir (çok büyük olanlar 1600 piksel genişliğe getirilir). İnternetten bir sayfa için resimler yalnızca herkese açık adreslerden getirilir: kendi bilgisayarınızda ya da özel bir ağda bulunan (yönlendirme yoluyla bile) dışarıda bırakılır ve sonuç kaçının bırakıldığını söyler. Kendisi sizin bilgisayarınızdan ya da ağınızdan gelen bir sayfa tüm resimleriyle getirilir.

**Şu olarak içe aktar**:

- **Metin belgesi** (varsayılan): başlıklar, paragraflar, alıntılar, listeler, tablolar (her satır için bir satır), program kodu, resim yazıları, kalın, italik ve bağlantılar, düzenleyebileceğiniz, not ekleyebileceğiniz, bölebileceğiniz ve derleyebileceğiniz sıradan bir belge olur.
- **Web sayfası belgesi**: makalenin resimleriyle birlikte, başvuru için salt okunur bir kopyası. Sayılmaz ya da derlenmez.

**Bir web sayfası belgesini görüntülemek** (0.81.3). Düzenleyici sayfayı basit biçimlendirilmiş metin olarak, adresiyle ve üstünde iki düğmeyle gösterir. **Kaydedilen Sayfayı Tarayıcıda Aç** sayfayı kaydedildiği haliyle web tarayıcınızda gösterir: {{APP}} bir kopyasını İndirilenler klasörünüze koyar (*Quillbinder web pages/<başlık>*); çünkü korumalı alanlı bir paket olarak kurulmuş bir tarayıcı (örneğin Kubuntu'da Firefox) {{APP}} programının kendi klasörlerini okuyamaz; düğmelerin altındaki satır kopyanın nerede olduğunu söyler ve bir sonraki tıklama onu değiştirir. Windows'ta kopya bunun yerine geçici bir klasörde yapılır ve {{APP}} kapandığında kaldırılır; böylece programın klasörünün dışına hiçbir şey yazılmaz. **Özgün Adresi Aç** sayfanın adresini açar; böylece sayfayı bugünkü haliyle görürsünüz; sayfanın adresi yoksa düğme kapalıdır. Scrivener'ın kaydettiği web sayfaları (her biri tek bir *.mht* dosyası) aynı şekilde gösterilir; Firefox gibi bir tarayıcı *.mht* dosyalarını kendisi açamaz; bu yüzden kopya, resimleri yanında olan düz HTML'dir.

Her iki durumda da sayfanın adresi belgenin **yer imi** (denetçi, Yer İmleri) olur, sayfanın kısa açıklaması **özeti** olur ve **notlar** adresi, siteyi, yazarı, yayın tarihini ve sayfayı ne zaman getirdiğinizi kaydeder.

Bir web tarayıcısından bir **bağlantıyı sürüklemek** Binder'daki bir klasöre, mantar panoya ya da anahat görünümüne, sayfayı doğrudan, penceredeki en son seçimlerinizle içe aktarır.

Bilmekte yarar var: sayfa bir tarayıcının yaptığı gibi, ama betikleri çalıştırılmadan getirilir. Metnini bir betikle oluşturan sayfalar çoğunlukla yine de onu verilerinde taşır; bu durumda o kullanılır (biçimlendirme ve resimler olmadan; bir ileti bunu söyler). Bir oturum açmanın ya da ücret duvarının arkasındaki sayfalar, oturum açmamış bir ziyaretçiye gösterdiklerini verir. Güvenli sayfalar (https) sisteminizde OpenSSL 3 gerektirir (her Ubuntu'nun parçasıdır). Bir PDF dosyasına giden bir web adresi bu yolla içe aktarılmaz: dosyayı indirin ve **Dosya > İçe Aktar > Dosyalar…** komutunu kullanın.

**Senaryolar.** Fountain dosyaları (*.fountain*) ve Final Draft dosyaları (*.fdx*) senaryo olarak içe aktarılır: paragraflar projenizin senaryo biçiminin öğelerini alır, belge senaryo modundadır ve dosyanın başlık sayfası belgenin notlarına gider. Kalın, italik ve altı çizili metin, notlar ve zorunlu sayfa sonları korunur; sahne numaraları, revizyon işaretleri ve çift diyalogun iki sütunlu görünümü korunmaz (iki konuşma art arda gelir). **Sahne başlıkları** ile **İçe Aktar ve Böl…**, bir senaryoyu başlığından adlanan sahne başına bir belgeye keser.

### Bir Scrivener projesini içe aktarmak

**Dosya > İçe Aktar > Scrivener Projesi…** bir Scrivener 3 ya da 2 projesini (*.scriv* klasörü; içindeki *.scrivx* dosyasını seçin) okur ve ondan yeni bir {{APP}} projesi oluşturur. Scrivener'ın projesi yalnızca okunur, asla değiştirilmez. 0.92.0'dan beri içe aktarmadan sonraki rapor, kopyalanamayan her dosyayı da (bir resim, bir PDF, bir ses dosyası, bir dizin kartının resmi, bir zihin haritası) adlandırır ve böyle bir dosya içe aktarılmış sayılmaz; önceden tek söz söylenmeden dışarıda bırakılırdı.

İçe aktarma şunları getirir: Binder yapısı, metinler, özetler, notlar, etiketler, durumlar, anahtar sözcükler, özel üst veri, kesim türleri, derlemeye dahil etme işaretleri, anlık görüntüler, yorumlar ve dipnotlar, satır içi açıklamalar, araştırma dosyaları (resimler, PDF'ler ve diğerleri), koleksiyonlar, proje yer imleri ve hedefler. Derleme ayarları içe aktarılmaz; bunları derleme penceresinde bir kez kurun. Yeni projenin metnini Markdown olarak mı RTF olarak mı saklayacağını seçin.

Başlıklar ve stiller: Scrivener dosyalarının içinde stillendirilmiş metni `<$Scr_Ps::0><$Scr_H::2>` gibi kodlarla işaretler. Bunlar metin değildir; {{APP}} bunları çıkarır. Bir başlık **Başlık 1** ile **Başlık 6** arasını alır ve Scrivener'ın paragraf stillerinden birine sahip bir paragraf, projenizde aynı adlı stili alır (**Başlık**, **Blok Alıntı**; Scrivener'ın **Centered Text** stili **Ortalanmış** olur). 0.81.3'ten beri Scrivener'ın karakter stillerinden birine sahip metin, projenizde o adı taşıyan karakter stilini alır ve projenizde o adda stili bulunmayan bir Scrivener stili, Scrivener'ın ona verdiği görünümle projenin bir stili olur. 0.14'ten önceki sürümlerle içe aktarılan projeler kodları metin olarak koruyordu; {{APP}} böyle bir proje açıldığında bunları çıkarır ve bunu durum çubuğunda söyler. Proje yer imleri 0.14'ten önce içe aktarılmıyordu; onları almak için Scrivener projesini yeniden içe aktarın.

Bir şey doğru gelmediyse, özgün proje dokunulmamıştır ve metni doğrudan Scrivener'dan kopyalayabilirsiniz.

### Scrivener biçiminde çalışmak

Bir kopya yapmak yerine {{APP}} doğrudan bir Scrivener 3 projesi üzerinde de çalışabilir; böylece daha sonra (örneğin başka bir bilgisayarda) Scrivener'da devam edebilirsiniz.

- **Açmak**: **Dosya > Proje Aç…** (başlangıç penceresinde **Başka Aç…**) ve *.scriv* klasörünün içindeki *.scrivx* dosyasını seçin. {{APP}} ilk seferde sorar: **Scrivener Biçiminde Çalış** (değişiklikleriniz *.scriv* içine kaydedilir), **Dönüştür…** (yukarıdaki içe aktarmada olduğu gibi ondan kendi {{APP}} projesi yapılır) ya da **İptal**. Bundan sonra proje soru sorulmadan açılır; **Dosya > Son Kullanılanlar** üzerinden de.
- **Yeni projeler**: Yeni Proje penceresinde **Biçim: Scrivener 3 projesi (.scriv)** seçin.
- **Kaydetmek**: her kayıt (otomatik olan da) *.scriv* içine yazar. Yalnızca değiştirdiğiniz yazılır; dokunmadığınız belgeler Scrivener'ın yazdığı dosyaları korur. Pencere başlığı *(Scrivener biçimi)* der ve **Dosya > Projeyi Dosya Yöneticisinde Göster** ile yedekler *.scriv* klasörünü alır.
- **Büyük projeler** (0.60.5'ten beri): siz yazarken otomatik kayıt *.scriv* içine yalnızca değiştirdiğiniz metinleri, notları ve özetleri (ve Binder değiştiyse onu) koyar; böylece yüz binlerce sözcükle bile kısa kalır. Scrivener'ın arama dizini, anlık görüntüler ve resimler gibi geri kalan her şey başka bir programa geçtiğinizde, **Dosya > Kaydet** (**Ctrl+S**) kullandığınızda ya da projeyi kapattığınızda izler; bu yüzden projeyi Scrivener'da açmadan önce burada kapatın ya da önce başka bir programa geçin.
- **Aynı anda bir program**: Scrivener projeyi açık tutuyorsa (ya da düzgün kapatılmadıysa), {{APP}} bunu söyler ve açmadan önce sorar. Projeyi öbür programda açmadan önce birinde kapatın. Proje, burada en son açık olduğundan beri Scrivener'da (ya da bir eşitleme hizmeti tarafından) değiştirildiyse, {{APP}} onu yeniden okur ve kaç belgenin değiştiğini durum çubuğunda söyler; bu arada Scrivener'da değiştirilen bir belgenin üzerine hiçbir zaman daha eski bir metin yazılmaz.

*.scriv* içine giden her şey: Binder, metinler (RTF olarak, resimleriyle), özetler, notlar (resimlerin, PDF'lerin, ses ve videonun notları da; 0.81.3'ten beri), etiketler, durumlar, kesim türleri, **Derlemeye Dahil Et**, yer imleri (projenin ve her belgenin; web adreslerine ve dosyalara da), son tarihli proje hedefleri, belge hedefleri, yazma geçmişi, mantar pano (her klasörün düzeni ve serbestçe yerleştirilen kartların yerleri), derlenen dosyanın başlığı, yazarı ve dili (Scrivener'ın derleme ayarlarında), anlık görüntüler, yorumlar ve dipnotlar, satır içi açıklamalar ve dipnotlar, paragraf ve karakter stilleri, anahtar sözcükler, özel üst veri alanları ve değerleri, koleksiyonlar, kayıtlı aramalar (arama koleksiyonları, 0.90.0'dan beri), dizin kartlarının resimleri (0.90.0'dan beri), resimler, PDF'ler, ses ve video, web sayfaları, simgeler (Scrivener'ın adlarıyla; kendi simgeleriniz *Icons* klasöründe) ve zihin haritaları (belgenin dosyalarının yanında, Scrivener'ın dokunmadığı ek bir dosya olarak). Her şey Scrivener 3 for Windows'un kullandığı biçimde yazılır:

- Anahtar sözcükler, özel üst veri ve koleksiyonlar Scrivener'ın numaralarını ve adlarını korur (0.81.2'den beri); bu yüzden iki programda aynıdırlar.
- Stiller (0.81.3'ten beri): değiştirdiğiniz bir metnin stilleri içinde Scrivener'ın işaretlediği gibi işaretlenir; böylece Scrivener aynı stilleri gösterir. {{APP}} içinde yaptığınız bir stil, bir metin onu ilk kullandığında görünümüyle birlikte Scrivener projesinin stillerine eklenir.
- Satır içi açıklamalar ve dipnotlar (0.81.3'ten beri) satır içi kalır, açıklamalar renkleriyle (önceden, değiştirdiğiniz bir metin bunları Scrivener'a bağlantılı yorumlar ve dipnotlar olarak veriyordu).
- Web sayfaları (0.81.3'ten beri): {{APP}} içinde kaydettiğiniz bir sayfa Scrivener'ın kendi kayıtlı web sayfası olarak (resimleriyle tek bir *.mht* dosyası) girer ve adresini korur. Scrivener'ın bir türü olmayan diğer dosyalar, dosyayı adlandıran metin belgeleri olarak, dosya yanlarında olmak üzere girer; {{APP}} bunları olduğu gibi geri okur.
- 0.81.2'den önceki sürümlerin bu tür metin belgelerine dönüştürdüğü ses, video ve web sayfaları, bir sonraki kayıtla yeniden Scrivener'ın kendi ses, video ya da web sayfası belgeleri olur.
- Metinler (0.90.0'dan beri), Scrivener'ın içlerinde tuttuğu her şeyle geri yazılır: sıfırın altındaki sağ girintiler, numaralandırmalarıyla listeler, bağlantıların altı çizgisi, bir dosyaya bağlanan resimler, yüzlerinin adıyla anılan yazı tipleri (*Cambria-BoldItalic* gibi) ve verisi eksik bir resmin yeri. Yorumlar ve dipnotlar, siz değiştirmediğiniz sürece kimliklerini, kutularının katlanıp katlanmadığını ve kendi biçimlendirmelerini korur.
- Mantar pano (0.90.0'dan beri): serbestçe yerleştirdiğiniz kartlar yerlerini iki programda da korur. Scrivener bir yeri kart genişlikleriyle tutar; {{APP}} bunu kendi kart boyutuna çevirir; böylece düzen her programın kartları hangi boyutta gösterdiğinden bağımsız olarak aynı kalır. Her klasör düzenini (serbest ya da ızgara) Scrivener'da olduğu gibi korur: bir klasörün mantar panosu için **Serbest Biçim**'i ya da ızgarayı seçmek o klasör için hatırlanır.
- Etiketler ve durumlar (0.90.0'dan beri): yeni belgelerin varsayılan etiketi ve durumu ile Scrivener'ın "No Label" ve "No Status" için verdiği adlar, projedeki gibi kalır.
- Kayıtlı aramalar (0.90.0'dan beri): Scrivener'ın kayıtlı bir araması {{APP}} içinde bir arama koleksiyonudur ve yaptığınız ya da değiştirdiğiniz bir arama koleksiyonu *.scriv* içine kayıtlı bir arama olarak girer. Scrivener'ın son arama sonuçları listesi Scrivener'a ait kalır. Sözcüklerden herhangi biri için kayıtlı bir aramayla denendi; Scrivener'ın diğer türleri (tüm sözcükler, tam öbek, düzenli ifade, yalnızca derlemeye dahil edilen ya da edilmeyen belgeler) nasıl adlandırdığı henüz gerçek bir projede görülmedi; bu yüzden bu türlerin kayıtlı bir araması Scrivener'da tüm sözcükler için bir arama olarak çıkabilir. Aramanın belgelerde nerede baktığı karşıya geçen şeyin parçası değildir: Scrivener'dan okunan kayıtlı bir arama her yere bakar.
- Bir dizin kartının resmi (0.90.0'dan beri): özet yerine resim gösteren bir kart onu iki programda da korur; {{APP}} içinde kaldırmak ya da koymak Scrivener'a ulaşır.

Yalnızca {{APP}} tarafında kalanlar (*~/.local/share/{{APP}}/ScrivenerProjects* içindeki bir çalışma kopyasında): senaryo biçimleri, derleme ayarları (başlık, yazar ve dil dışında), nesneler (Bölüm 7) ve pencere düzeni; *.scriv* içinde bunlardan zaten ne varsa olduğu gibi bırakılır.

Bir Scrivener projesi ilk kez açıldığında ya da içe aktarıldığında Binder'ı Scrivener'da açık olan klasörleri ve orada seçili belgeleri gösterir (0.90.0; {{APP}} Scrivener'ın kendi pencere ayarlarını hiçbir zaman yazmaz). Scrivener biçimindeki bir projeyi 0.81.2 ya da sonrasıyla ilk kez açtığınızda {{APP}} *.scriv* dosyasını yeniden okur ve çalışma kopyasının anahtar sözcüklerini, özel üst verisini ve koleksiyonlarını projeninkilerle birleştirir; böylece eski bir sürümle yaptığınız hiçbir şey kaybolmaz; bir sonraki kayıt bunları *.scriv* içine koyar. 0.90.0 ya da sonrasıyla ilk açılış, yer imleri, belge hedefleri, son tarih, yazma geçmişi ve mantar pano için de aynısını yapar.

Çalışma kopyasına yeni belgeler yazıldıktan sonra *.scriv* içine bir kayıt yarıda kesilirse (bir çökme, dolu bir disk), bir sonraki açılış bu belgeleri korur ve bunu söyler; bir sonraki kayıt onları *.scriv* içine koyar. Scrivener'da silinen belgeler silinmiş kalır.

**Dosya > Farklı Kaydet…** (**Ctrl+Shift+S**) açık projeyi başka bir adla ya da başka bir yere kaydeder ve bir projenin biçimini bu şekilde değiştirir. İletişim kutusunda adı yazın ve yeri seçin; **dosya türü** biçimi belirler: *Scrivener 3 Projesi (\*.scriv)* ya da *{{APP}} Projesi (\*.qbx)* (*.scriv* ya da *.qbx* ile biten bir ad kendiliğinden belirler). Bir proje bir klasördür: program onu addan yapar (*Ad.scriv* ya da içinde *Ad.qbx* bulunan *Ad.quillbinder*). Zaten var olan bir ad reddedilir.

- Bundan sonra pencere yeni proje üzerinde çalışır ve sonraki her kayıt oraya gider. Projenin başlığı yazdığınız addır. Henüz kaydetmediğiniz değişiklikler yalnızca yeni projeye girer.
- Özgün, o andaki haliyle diskte kalır (kopya olarak saklayın ya da silin); aynı zamanda **Dosya > Son Kullanılanlar** içinde, yeni projenin altında durur. Kilit dosyası, pencere ondan ayrıldığında kaldırılır ve yeni proje kendi kilit dosyasını alır.
- Yeni proje kendi **yedekleriyle** başlı başına bir projedir: yeni bir seri başlatır ve şimdiye dek yapılan yedekler özgünle kalır (Bölüm 14, Yedekler). Scrivener biçimindeki bir proje bunun için *.scrivx* içinde yeni bir tanımlayıcı alır.
- Farklı Kaydet, bir biçimden aynı biçime (yeni bir ad ya da yer) ve diğerine (0.92'ye kadar "Proje Biçimi" menüsünün yaptığı geçiş) aynı şekilde çalışır.

**Dosya > Dışa Aktar > Scrivener Projesi…** Scrivener biçiminde bir **kopya** yazar ve aynı menüde altındaki girdi, **{{APP}} Projesi…**, {{APP}} programının kendi biçiminde bir kopya yazar; ikisi de pencereyi özgünde bırakır: üzerinde çalıştığınız projede hiçbir şey değişmez (0.93.0'dan önce bunlar "Proje Biçimi" menüsünde "... olarak Kopya Kaydet" idi).

### Belgeleri dışa aktarmak

**Dosya > Dışa Aktar > Dosyalar…** seçili belgeleri diğer programlar için kaydeder. Bitmiş bir el yazması için Derle'yi kullanın; tek tek belgeleri birine vermek ya da {{APP}} dışında bir kopya tutmak için Dışa Aktar'ı kullanın.

1. Binder'da dışa aktarılacak belgeleri ve klasörleri seçin.
2. **Dosya > Dışa Aktar > Dosyalar…** komutunu seçin.
3. **Metin dosyalarını şu olarak dışa aktar**: Zengin Metin (.rtf), Word (.docx), Markdown (.md), düz metin (.txt), Fountain ya da Final Draft (senaryolar için Fountain önceden seçilidir). Resimler, PDF dosyaları, web sayfaları, ses ve video olduğu gibi dışa aktarılır; biçim bu durumda yalnızca notları için geçerlidir.
4. **Üst Veri**: **Üst veriyi dışa aktar** her belgenin yanına, başlığı, türü, etiketi, durumu, anahtar sözcükleri, derlemeye dahil olup olmadığı, kesim türü, sözcük sayısı, hedefi, oluşturma ve değiştirme tarihleri, özel üst verisi ve yer imleriyle "*ad* (metadata).txt" adlı bir metin dosyası yazar. **Özetleri dışa aktar** özeti ona ekler (üst veri dışa aktarılmıyorsa "*ad* (synopsis).txt" yazar).
5. **Seçenekler**: **Alt belgeleri dahil et** (klasörler klasör olur), **Notları dışa aktar** ("*ad* (notes)", 3. adımın biçiminde; senaryolar için düz metin), **Yorumları ve satır içi açıklamaları dışa aktar** (temiz metni teslim etmek için işareti kaldırın; yorumlanan sözcükler kalır, dipnotlar her zaman kalır), **Anlık görüntüleri dışa aktar** (anlık görüntü başına bir dosya içeren, tarih ve başlıkla adlandırılmış bir "*ad* (snapshots)" klasörü).
6. **Konum**: kaydedilecek klasör. Birden çok belge seçiliyse, adını verdiğiniz yeni bir klasöre girerler (projenin başlığı önerilir; klasör varsa bir numara eklenir).
7. **Dışa Aktar** düğmesine tıklayın. Klasör dosya yöneticisinde açılır.

Seçimler bir sonraki dışa aktarma için hatırlanır.

Hızlı yol: **belgeleri Binder'dan** dosya yöneticisine ya da masaüstüne **sürükleyin**. Metin belgeleri RTF dosyaları olarak kaydedilir (ya da Word, Markdown veya düz metin: **Dosya > Seçenekler > Davranışlar**), resimler ve diğer dosyalar olduğu gibi, bir klasör de belgeleri içinde olan bir klasör olarak. Dosya yöneticisi taşımak mı kopyalamak mı istediğinizi sorarsa ikisi de uygundur: belgeler projenizde kalır.

### Karşılaştır ve aktar: bir Word ya da LibreOffice dosyası ile proje

Bir el yazması bir editörden geri geldiğinde ya da üzerinde Word'de veya LibreOffice'te çalışmaya devam ettiğinizde, **Dosya > Karşılaştır ve Aktar > Dosyayla Karşılaştır…** o dosya ile projeniz arasındaki farkları bulur ve seçtiğiniz tarafı devralır.

1. Dosyayı seçin: bir Word dosyası (*.docx*) ya da bir LibreOffice dosyası (*.odt*). Eski Word dosyaları (*.doc*), RTF, düz metin ve Markdown dosyaları da karşılaştırılabilir, ama yalnızca okunur: metinleri projeye girebilir; dosyanın kendisi değiştirilemez.
2. {{APP}} dosyayı tüm projeyle değil, **Derle'nin projeden ürettiğiyle** karşılaştırır: derleme grubunun (derleme penceresinde başka bir grup seçilmedikçe Taslak) **Derlemeye Dahil Et** işaretli belgeleri, Binder sırasıyla. Ön ve son sayfalar, Araştırma klasörü ve Derlemenin dışında bırakılan belgeler sayılmaz. Dosyanın metninin %65'inden azı onda bulunursa, {{APP}} bunu söyler ve yine de karşılaştırıp karşılaştırmayacağını sorar (**Yine de Karşılaştır**); dosya başka bir projeye ait olabilir ya da Derle dosyada olanı dışarıda bırakıyor olabilir. Bir sözcük, derlenen metinde de bulunan dört sözcüklük bir dizinin içinde yer alıyorsa bulunmuş sayılır; bu yüzden birkaç değişen sözcük bir dosyayı ilgisiz yapmaz.
3. **Karşılaştır ve Aktar** penceresi farkları gösterir. Güncellenecek tarafı seçin:
   - **Projeyi dosyadan güncelle**: değişiklikler belgelerinizde yapılır. Bundan önce değişen her belge, "Before Compare and Transfer (*dosya adı*)" başlıklı bir anlık görüntü alır (**Belgeler > Anlık Görüntüler**); böylece her zaman geri dönebilirsiniz. Yalnızca dosyada olan metin yeni bir belge olur (dosyadaki başlığından adlanır) ve kendinden önceki belgenin ardına yerleştirilir.
   - **Dosyayı projeden güncelle**: seçtiğiniz dosya olduğu gibi kalır. {{APP}} yanına "*ad* (Quillbinder).docx" adlı bir kopya yazar (bu ad alınmışsa bir numarayla). Bir Word dosyasında her değişiklik, "Quillbinder" yazarının **izlenen bir değişikliğidir**: kopyayı Word'de ya da LibreOffice'te açın ve her değişikliği orada kabul edin ya da reddedin (Word'de Gözden Geçir sekmesinde; LibreOffice'te Düzenle, Değişiklikleri İzle, Yönet ile). Bir LibreOffice kopyası (*.odt*) değişiklikleri işaretsiz, doğrudan alır.
4. Görünümde **yeşil** metin içeri gelir, **kırmızı** üstü çizili metin gider ve **mavi** metin öbür tarafın kalın, italik ya da altı çizili biçimini alır. Gri paragraflar, bir değişikliğin çevresindeki, nerede olduğunu göstermek için değişmemiş metindir. Her yerin bir **al** ya da **atla** bağlantısı vardır; bir tıklama onu değiştirir.
5. **... Projeye Aktar** ya da **... Dosyanın Bir Kopyasına Yaz** düğmesine tıklayın. Bir ileti nelerin yapıldığını söyler; dosya için **Dosyayı Aç** kopyayı açar.

**Bölüme Göre** (**Dosya > Karşılaştır ve Aktar > Bölüme Göre**, pencerede bir işaret kutusu olarak da) belgeleri solda, her biri bir işaret kutusu ve nelerin farklı olduğuyla ("2 yer", "yalnızca dosyada", "dosyada yok") listeler. Bir belgeyi olduğu gibi bırakmak için işaretini kaldırın; bir tıklama değişikliklerini sağda gösterir. Bölüme Göre olmadan tüm değişiklikler alt alta gösterilir.

**Adım Adım** (**Dosya > Karşılaştır ve Aktar > Adım Adım**, pencerede de) her seferinde bir değişikliği gösterir: onu **Al** ya da **Atla**, ya da **Geri** gidin. Sonunda pencere kaç değişikliğin alınacağını söyler; sonra Aktar'a tıklayın. İki seçim de bir sonraki sefer için hatırlanır.

Karşılaştırılanlar: sözcükler ile kalın, italik ve altı çizili. Dosyanın her paragrafı sözcüklerine sahip belgeyle eşleştirilir; böylece bir sahne dosyada taşınmış olsa da bulunur (bu durumda "dosyada başka bir yerde" olarak işaretlenir; Binder'ın sırası değiştirilmez). Dosyanın bir belgenin metni olmayan başlıkları, örneğin Derle'nin yaptığı bölüm başlıkları, karşılaştırılmaz.

Başlangıçta işaretli olanlar: bir belgenin içindeki her değişiklik. Yalnızca bir tarafta olan metin, en az 100 sözcüğü varsa işaretlenir (dosyanın yeni bir bölümü yeni bir belge olur; dosyada eksik bir sahne kopyaya eklenir). Siz işaretlemedikçe bütün bir belge hiçbir zaman Çöp Kutusu'na taşınmaz ve bütün bir bölüm dosyadan hiçbir zaman çıkarılmaz.

Sınırlar: yeni paragraflar yanındaki paragrafın görünümünü alır. Tablolardaki metin diğer paragraflar gibi karşılaştırılır (bir tablonun kendisi hiçbir zaman çıkarılmaz) ve alanlar gösterdikleri metinle sayılır. Üst ve alt bilgiler, metin kutuları ve dosyanın dipnotlarının ve yorumlarının metni karşılaştırılmaz ve olduğu gibi kalır; projenin yorumları ve dipnotları projede kalır ve dosyaya yazılmaz. Dosya karşılaştırmadan sonra değiştirildiyse kopya yazılmaz; yeniden karşılaştırın.

### Yazdırma

**Dosya > Geçerli Belgeyi Yazdır…** (**Ctrl+P**) düzenleyicinin gösterdiğini yazdırır:

- **Bir metin** (bir belge ya da birleşik metinde birkaçı): biçimlendirmesi ve resimleriyle metin, her belgenin metninin üstünde başlığı ve sayfanın altında dipnotlar. Klavye bir referans bölmesindeyse, referans bölmesinin belgesi yazdırılır.
- **Mantar pano**: gösterilen sırayla dizin kartları, bir sayfaya sığabildiği kadar (kesilecek çizgilerle) ya da dizin kartı kâğıdı için her sayfada bir kart.
- **Anahat**: anahtarın gösterdiği gibi satırlar ve sütunlar, sütun başlıkları her sayfanın üstünde.
- **Bir resim** (sayfanın izin verdiği kadar büyük), **bir PDF dosyası** (sayfaları) ya da **bir web sayfası** (metni).

Yazdırma penceresi yazıcıyı, sayfaları ve kopya sayısını seçer. **Seçim** yalnızca seçili metni ya da yalnızca seçili dizin kartlarını veya anahat satırlarını yazdırır. Çıktıyı bunun yerine PDF dosyası olarak kaydetmek için yazıcı olarak **Dosyaya Yazdır (PDF)** seçin.

**Dosya > Baskı Önizleme…** önce sayfaları gösterir; önizlemenin yazıcı düğmesi onları yazdırır, **Kapat** (ya da **Esc**) önizlemeyi kapatır.

**Dosya > Yazdırma Ayarları…** işlerin kâğıda nasıl gideceğini belirler. Pencere, düzenleyicinin gösterdiği için olan sekmede açılır; **Önizleme…** siz **Tamam** düğmesine tıklamadan önce sonucu gösterir. Ayarlar projeye aittir; yeni projeler için de varsayılan yapmak üzere **Bu ayarları yeni projeler için de kullan** işaretini koyun.

- **Sayfa**: kâğıt boyutu ve yönü (**Sayfa Görünümü Kâğıdı** düğmesi, **Görünüm > Metin Düzenleme > Sayfa Boyutu ve Kenar Boşlukları…** bölümündeki kâğıdı ve kenar boşluklarını devralır), kenar boşlukları ve her biri sol, orta ve sağ bölümlü üst ve alt bilgi. Yer tutucular: `<$title>` (yazdırılan), `<$projecttitle>`, `<$author>`, `<$date>`, `<$p>` (sayfa numarası) ve `<$pages>` (sayfa sayısı). Temiz bir ilk sayfa için **İlk sayfada da üst ve alt bilgi** işaretini kaldırın.
- **Metin**: metinle birlikte nelerin yazdırılacağı (her belgenin başlığı, altında özet, metinden sonra belge notları, sayfanın altında not olarak yorumlar ve satır içi açıklamalar, her sayfanın altı yerine metnin sonunda dipnotlar, vurgular); yazı tipleri (düzenleyicideki gibi ya da seçeceğiniz **Tek yazı tipi**); ve satır aralığı (düzenleyicideki gibi, tek, 1,5 satır ya da düzeltmeler için yer bırakan çift). Senaryo belgeleri senaryo düzenini korur.
- **Dizin Kartları**: kart boyutu (3 x 5 inç, 4 x 6 inç, 5 x 8 inç, A7, A6 ya da kendi boyutunuz), bir sayfaya sığabildiği kadar kart ya da her sayfada bir kart, her kartın neyi gösterdiği (başlık, özet, özet yoksa metnin ilk satırları, resimler, üstte şerit olarak etiket rengi, durum, anahtar sözcükler, bir dizin kartının çizgileri, her kartın çevresinde bir çizgi) ve yazı tipi.
- **Anahat**: özetler (anahtarın gösterdiği gibi, her zaman ya da hiçbir zaman), kapalı grupların içindeki satırlar da, her sayfanın üstünde sütun başlıkları, satırlar arasında bir çizgi, etiket renginde bir nokta ve yazı tipi. Çok sayıda sütunla, **Sayfa** sekmesindeki **Yatay** onlara daha çok yer verir.

Geçerli Belgeyi Yazdır, bir belgeyi üzerinde çalıştığınız gibi yazdırır. Bir derleme biçiminin görünümüne sahip bitmiş bir el yazması için derleyin (Bölüm 13) ve PDF ya da Word dosyasını yazdırın.

---

## 15. Yedekler

{{APP}} bir projeyi her kapattığınızda bir yedek (projenin tamamının ZIP dosyası) yapar ve proje başına son beşini tutar. Bunu **Dosya > Seçenekler > Yedekleme** altında değiştirin; orada yedek klasörünü de seçebilirsiniz (en iyisi başka bir sürücüde ya da eşitleme programınızın kopyaladığı bir klasör).

0.91'den beri tüm projelerin yedekleri doğrudan yedek klasöründe durur ve her biri projesinin adı, tarih ve saatle adlandırılır. 0.92.0'dan beri ad ayrıca projenin **kimliğini** (ID) taşır, bir "#" işaretinden sonra beş karakter (*My Novel #K7T2M bak 2026-11-20 15-40-00.zip*). Kimlik projeye aittir, bilgisayara değil: projenin açıldığı her bilgisayarda aynıdır ve projeyi yeniden adlandırdığınızda ya da klasörünü taşıdığınızda kalır. Bir projenin yedekleri, başlığını ve kimliğini taşıyan dosyalardır ve onun için yalnızca onlar sayılır ve temizlenir; bu yüzden aynı başlıklı iki proje kendi yedeklerini ve kendi tutulan sayısını korur. Programın kendisinin yaptığı bir kopya ya da yeni bir proje, kendi kimliğiyle başlı başına bir projedir: **Dosya > Farklı Kaydet…** ve **Dosya > Dışa Aktar** menüsünün {{APP}} Projesi girdisi, **Yedekten Geri Yükle** ile geri yüklenen bir proje (ayrıca kendi başlığını alır) ve kendi şablonunuzdan yapılan bir proje. Program dışında yapılan bir kopya (elle kopyaladığınız bir klasör, elle açtığınız bir yedek), özgününün kimliğini taşır: böyle bir kopyaya kendi başlığını verin. Scrivener biçimindeki bir proje kimliğini, Scrivener'ın *.scrivx* içinde tuttuğu tanımlayıcıdan alır. Yedek klasörünüz bir kuruluşun OneDrive'ındaysa, yöneticisi dosya adlarında "#" işaretini yasaklamış olabilir; ev kullanımı için OneDrive buna izin verir. 0.92.0'dan önceki bir sürümün yaptığı yedeklerin adında kimlik yoktur: temizlik onları asla silmez, **Yedekten Geri Yükle** yine de listeler ve artık gerek duymadığınızda kendiniz silersiniz. Bir projenin başlığını değiştirdiğinizde, eski başlıkla yapılan yedekler siz silene kadar kalır. 0.70 ile 0.90 sürümleri her projenin yedeklerini yedek klasörünün içinde kendi klasöründe tutuyordu (*My Novel 1a2b3c4d*); böyle bir klasör olduğu gibi bırakılır, orada artık hiçbir şey eklenmez ya da silinmez ve **Yedekten Geri Yükle** yine de yedeklerini listeler. Bir yedek pencerenin yanında çalışır: bir an sonra bir ilerleme penceresi ne kadar yapıldığını gösterir ve onu iptal edebilir. Projeyi kimin açtığı (bilgisayarın ve oturumun adını taşıyan kilit dosyası) yedeğe girmez.

- **Her elle kayıtla yedekle** (**Dosya > Seçenekler > Yedekleme**) ile her **Dosya > Kaydet** (**Ctrl+S**) de bir yedek yapar. Aynı saniye içindeki iki yedek bir numara alır ("... (2)"). Bu yedek yapılamadığında, eksik olduğunda ya da yedek klasörünün dışında beklemek zorunda kaldığında, metnin yanındaki bir bildirim bunu söyler (0.92.0; önceden yalnızca durum çubuğu, birkaç saniyeliğine söylerdi). Metniniz her halükârda kaydedilir. Bildirim, program kapatılana kadar neden başına bir kez gelir; durum çubuğu her kayıtta söyler.
- **İleti satırı**, Binder'ın altında kısa iletileri tek tek, 5 saniye (bağlantılıysa 10) ya da siz tıklayana kadar gösterir; diğerleri bekler. Eklentiler de kullanır (örneğin Çeviri ve Düzeltme Okuması ya da Konuşma Yazıya Dökme modelini 15 dakika kullanılmadıktan sonra boşalttığında "Model unloaded, RAM usage minimal"; süre seçeneklerindedir). **Yardım > Sorun Giderme ve Geliştirme > Sistem Bilgisi** programın tuttuğu belleği adlandırır.
- **Kurulu olmayan eklentiler.** Eklenti yokken ya da kapalıyken **Düzenle > Yazım Araçları** "Çevir ve Düzeltme Oku" ile "Konuşma Yazıya Dökme" girdilerini listeler; her biri **Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi** bölümünü açan bir düğmesi olan bir bildirim gösterir.
- **Dosya > Yedekle > Şimdi Yedekle** hemen bir yedek yapar. 0.94.0'dan beri başarı, Binder'ın altındaki ileti satırında söylenir ("Backup saved." ve yedeğin klasörünü açan **Klasörü Göster** bağlantısı; ipucu dosyayı adlandırır; bağlantılı bir ileti 10 saniye kalır); hiçbir kutu Tamam için beklemez. Bir kutu yalnızca bir şey başarısız olduğunda ya da yedek yapıldığı halde bazı dosyalar okunamadığında ya da eski bir yedek silinemediğinde gelir.
- **Bir kapanışın ya da sonlandırma komutunun yapılmasını engellediği yedek** not edilir ve bir sonraki açılışta yapılır (0.81). 0.93.0'dan beri not, o yedek yapılana kadar kalır (yedek klasöründe ya da yukarıda anlatıldığı gibi yanında bekleyerek): yedeği başarısız olan bir açılış, notu bir sonraki açılışa bırakır. **Temizliğin silemediği eski bir yedek** (kilitli ya da korumalı bir dosya) kalır ve bir ileti dosyayı ve nedenini bir kez adlandırır; elle silin. Siz yazmaya devam ederken bu ileti, hiçbir tuş almayan, metnin yanındaki küçük bir bildirimdir.
- **Şuraya Yedekle…** bir yedeği seçtiğiniz yere, örneğin bir USB belleğe, kaydeder.
- **Yedekten Geri Yükle…** (0.70) açık projenin yedeklerini tarihleri, boyutları ve eksiksiz olup olmadıklarıyla listeler; birini seçin ve **Geri Yükle ve Aç**: projenin yanında yeni bir klasöre, *… (restored 2026-11-10 14-03)*, açılır, başlığı "(restored …)" der ve orada açılır. Açık olan projeniz değişmez. Liste, bu projenin kimliğini taşıyan yedekleri, kimliği olmayanları (0.92.0'dan önce yapılanlar) ve aynı başlıklı başka bir projeninkileri tutar; listedeki dosya adı hangisinin hangisi olduğunu gösterir; böylece hiçbir şey erişim dışında kalmaz. 0.93.0'dan beri programın kendi çalışma klasöründe bekleyen yedekleri de listeler (yedek klasörüne ulaşılamazken yapılanlar; yedek klasörüne bir sonraki yedek onları oraya koyar): bunlar "waiting (not yet in the backup folder)" olarak işaretlenir ve aynı şekilde geri yüklenir. 0.81'den beri her boyutta geri yükleme çalışır (2 GB ve üstü yedekler, videolar gibi 1 GB'ı aşan dosyalar, 65.535'ten fazla dosya), her dosya yazılırken denetlenir, sürücüye sığmayan bir geri yükleme başlatılmaz, bir ilerleme penceresi onu iptal edebilir ve dosyalar geri yüklenemediğinde ileti her birini nedeniyle (dolu bir disk, bozuk bir girdi) adlandırır. 0.93.0'dan beri iletinin üç biçimi vardır: **geri yüklendi** (her şey orada), **eksik geri yüklendi** (öyle başlıklı, bir uyarı işaretiyle, eksik dosyalar nedeniyle adlandırılmış: güvenmeden önce geri yüklenen projeye bakın) ve **geri yüklenmedi**; geri yüklenen projenin başlığı "(restored …)" olarak işaretlenemediğinde ileti bunu söyler.
- **Eski Yedekleri Yedek Klasörüne Taşı…** (0.93) 0.70 ile 0.90 sürümlerinin yedek klasörünün içinde kendi klasöründe tuttuğu yedekleri (*My Novel 1a2b3c4d*) adlarıyla yedek klasörünün kendisine taşır; böylece diğerleriyle birlikte dururlar. Yalnızca açık projenin yedeklerini içeren böyle bir klasörü olduğu sürece açıktır. Önce sorar ve kaç tane olduğunu söyler; adı yedek klasöründe zaten bulunan bir yedeğin üzerine hiçbir zaman yazılmaz ve olduğu yerde kalır; rapor nelerin taşındığını ve nelerin (nedenle birlikte) bırakıldığını söyler ve içinde hiçbir şey kalmadığında eski klasör kaldırılır. Bu yedeklerin adında kimlik yoktur; bu yüzden temizlik onları, öncekinde olduğu gibi, hiçbir zaman saymaz ya da silmez; **Yedekten Geri Yükle** onları her iki yerde de listeler.
- **Yedek Klasörünü Göster** yedek klasörünü açar.

**Bir yedeğin bir araya getirildiği yer (0.90.3'ten beri).** Bir yedek, programın kendi klasöründe, *~/.local/share/{{APP}}/backup-work* (Windows'ta program klasöründe *data\backup-work*) içinde paketlenir ve denetlenir ve yalnızca bitmiş dosya yedek klasörüne tek adımda ve son adıyla konur: iki klasör aynı sürücüdeyse oraya taşınarak, aksi halde doğrudan son adına kopyalanarak ve özgünle karşılaştırılarak. Böylece bir eşitleme programının izlediği, bir USB bellekte ya da bir ağ sürücüsündeki yedek klasörü hiçbir zaman yarım yazılmış bir yedek ya da geçici bir dosya içermez ve yarıda kesilmiş bir yedek hiçbir zaman bitmiş gibi görünmez. (0.90.3'ten önce yedek yedek klasörünün içinde *… .zip.part* olarak yazılır ve orada yeniden adlandırılırdı.) Çalışma klasörü, hizmet verdiği yedek klasörünün küçük bir notu dışında, yedekler arasında boştur.

**Yedek klasörüne ulaşılamadığında** (takılı olmayan bir sürücü, yapılamayan ya da yazılamayan bir klasör, yer olmaması), yedek yine de yapılır ve çalışma klasöründe bekler. Bir ileti bunu söyler ve her iki klasörü adlandırır ve projenin bir sonraki yedeği, kendisininkinden önce bekleyen yedekleri yerine koyar. Yedeklerin tutulduğu sayı kadarı bekler. Yazılamayan bir klasöre **Şuraya Yedekle…** yalnızca nedenini söyler; başka bir klasör seçin.

**Bir eşitleme programının yaptığı kopyalar.** Yalnızca {{APP}} programının yedeklerini adlandırdığı gibi adlandırılan dosyalar (*Title #K7T2M bak 2026-11-20 07-56-13.zip*, uygulanıyorsa "(2)" ya da "(incomplete)" ile) yedek sayılır ve temizlik tarafından silinebilir. Bir projenin yedek klasöründeki diğer dosyalara dokunulmaz ve sayılmazlar: örneğin bir eşitleme programının bir dosyayla sorun yaşadığında yaptığı kopyalar (*… -MyComputer-safeBackup-0001.zip*, *… (conflicted copy).zip*) ya da elle yaptığınız bir kopya. (0.90.3'ten önce bu tür kopyalar tarihlerinin yedekleri olarak sayılırdı ve birkaç tanesi gerçek eski yedekleri dışarı itebilirdi.) **Yedekten Geri Yükle…** yine de bunları listeler; böylece bir kopyadan geri yükleyebilirsiniz; gerek duymadığınız kopyaları kendiniz silin.

Büyük projeler (4 GB'tan büyük, örneğin ses ve videolu) da doğru şekilde yedeklenir (0.20 sürümünden beri). Okunamayan bir dosya (örneğin izinleri nedeniyle ya da henüz bu bilgisayarda olmayan eşitlenmiş bir klasörün dosyası) dışarıda bırakılır; yedek yine de yapılır, ama *… (incomplete).zip* olarak adlandırılır, bir ileti eksik dosyaları adlandırır (bir proje açılıp kapanırken yapılan otomatik yedekler için de, 0.65) ve bunun yüzünden hiçbir eski yedek silinmez: tutulacak yedek sayısı yalnızca eksiksiz yedekleri sayar. Hangi yedeklerin en eski olduğunu adlarındaki tarih ve saat söyler; bu yüzden projeyi yeniden adlandırmak hangilerinin tutulduğunu hiç değiştirmez (0.81; önceden eskisinden önce sıralanan yeni bir başlık her yeni yedeği en eski saydırır ve hemen silinirdi). Temizlikten sonra program yeni yedeğin hâlâ orada olduğunu denetler. Eksik yedeklerden, tutulacak sayıya kadar en yenileri kalır (0.81). Projede başka bir yerdeki bir klasöre bağlantı olan bir klasör dosyalarıyla birlikte yedeklenir (0.81); hiçbir yere ya da projenin içine geri işaret eden bir bağlantı yedeği eksik yapar. Program, **Bir proje kapatıldığında yedek yap** açıkken bir sinyalle sonlandırılırsa (beklemeyen bir kapatma, bir sonlandırma komutu), yedek için zaman kalmaz; proje bir sonraki açıldığında yedek yapılır ve durum çubuğu bunu söyler (0.81). Proje her yedekten önce kaydedilir; bu kayıt başarısız olursa (dolu bir disk, yazılamayan bir klasör), hiçbir yedek yapılmaz ve bir ileti nedenini söyler; böylece bir yedek, siz bilmeden en son değişikliklerinizden yoksun kalmaz. Scrivener biçimindeki bir projenin yedeği *.scriv* klasörünü ve yanında {{APP}} programının çalışma kopyasını (*… - working copy of {{APP}}*), yalnızca {{APP}} programının bildiği anahtar sözcükler, stiller, nesneler ve diğer şeylerle birlikte tutar.

**Bir yedeği elle geri yüklemek için** (örneğin başka bir bilgisayarda): yedek klasörünü açın, ZIP dosyasını açın (sağ tıklayın > Çıkar) ve içindeki projeyi **Dosya > Proje Aç** ile açın.

---

## 16. Seçenekler

**Dosya > Seçenekler** (**Ctrl+,**) en üstte bölümler için bir simge sırası, solda bir sayfa (ya da sekme) listesi ve sağda seçenekler içerir. **Tamam** ya da **Uygula** düğmesine tıklayana kadar hiçbir şey değişmez. Sol altta **Yönet**, kullanımdaki profili bir dosyaya dışa aktarır ve bir profili içe aktarır (**Profili Dışa Aktar...**, **Profili İçe Aktar...**; örneğin ayarlarınızı başka bir bilgisayara taşımak için; eski bir seçenekler dosyası (*.qboptions*) profil olarak içe aktarılabilir; aşağıdaki "Profiller" bölümüne bakın), **Profilleri Yönet...** penceresini açar, geçerli görünümü bir tema dosyası olarak kaydeder (var olan bir dosya ancak yenisi tamamen yazıldığında değiştirilir; yazma başarısız olursa eski dosya olduğu gibi kalır ve ileti bunu söyler) ve temaları listeler; **Varsayılanlar** her seçeneği standarda geri döndürür (son projeleriniz, kendi temalarınız, düzenleriniz, araç çubukları ve kısayollarınız kalır).

**Her seçeneğin nerede tutulduğu.** Bir seçeneğin değerinin nerede tutulduğunu görmek için üzerine gelin: ipucu, açık projenin mi yoksa programın mı onu tüm projeler için tuttuğunu ve o yerin **sabit** mi (dil gibi yalnızca programın tutabildiği bir ayar) yoksa en üstteki **Ayarların geçtiği yer** ayarına (ve Dış Görünüm sayfasında **Şunun görünümünü göster ve değiştir** ayarına) **bağlı** mı olduğunu söyler. (0.90.1 her seçeneğin önünde yeşil bir P ya da mavi bir A gösteriyordu; 0.90.2 bunu ipucuna bırakır.) Küçük bir ekranda sayfalar kayar; böylece Tamam, İptal ve Uygula görünür kalır.

0.70'ten beri pencere **Tüm projeler** (programın ayarları) için açılır; yalnızca açık proje için bir şey ayarlamak üzere en üstte **Bu proje** seçeneğini seçin. Bir projenin kendi ayarları kaydetmeyi daha güvenli yapabilir ama asla daha az güvenli yapamaz: daha kısa otomatik kayıt gecikmesi geçerlidir, programın yaptığı bir yedek açık kalır, daha büyük yedek sayısı tutulur ve otomatik çıkış yalnızca programındır. Son kayıt, otomatik kayıt gecikmesi ne olursa olsun, bir değişiklikten en geç 10 saniye sonra yapılır.

- **Genel**: **Başlangıç** (son projeyi yeniden aç, masaüstünün dosya iletişim kutuları, programın **Dili**, bir sonraki başlangıçtan itibaren: English, Deutsch, Español, Français, Português (Brasil), Italiano, Русский, Bahasa Indonesia, Türkçe, Polski ya da Nederlands (yeni dokuzu makine çevirisidir, bir konuşan kişi denetleyene kadar listede böyle işaretlidir; kılavuz, hızlı başlangıç kılavuzu ve karşılama projesi dili izler, bir dilde yoksa İngilizce kalır; ilk başlangıç, sistemin dili bunlardan biriyse onu alır ve ilk başlangıç sayfası aynı listeyi sunar); **Büyük sayıların basamaklarını grupla**: sözcük sayıları, hedefler ve istatistikler, binlerin arasında sisteminizin sayıları yazdığı gibi bir ayırıcıyla (44,825 ya da 44.825), işaret kaldırılırsa 44825); **Kaydetme** (otomatik kayıt aralığı, son projelerin sayısı, her Dosya > Kaydet ile anlık görüntüler); **Yazar Bilgileri** (başlık sayfaları için adınız, adresiniz, telefonunuz ve e-postanız; `<$author>`, `<$street>`, `<$city>`, `<$state>`, `<$zip>`, `<$country>`, `<$phone>`, `<$email>`, `<$address>` yer tutucuları tarafından kullanılır); yazım araçlarının seçenekleri (**Karalama Defteri**); **Paylaşılan Şablonlar** (proje şablonlarının bir başka klasörü, örneğin bilgisayarlarınız arasında eşitlenen bir klasör; şablonları Dosya > Yeni Proje içinde görünür); **Alıntılar** (**Biçim > Kaynakça/Alıntılar...** komutunun başlattığı program, örneğin Zotero ya da JabRef); **Ayırıcılar** (birleştirilen belgeler arasına ve bir belgeye eklenen karalama defteri notları arasına ne gireceği: tek bir satır sonu, boş bir satır, bir sayfa sonu, hiçbir şey ya da kendi metniniz); **Otomatik Çıkış** (belirli bir dakika sayısı boyunca tuş ya da fare girdisi olmadan kaydet ve kapat); **Uyarılar** (**Dahili Günlük Konsolunu Göster** programın bu oturumdaki iletilerini gösterir, **Tüm Uyarıları Sıfırla** "Bir daha sorma" ile yanıtlanan soruları yeniden sorar).
- **Düzenleme**: **Seçenekler** (düzenleyicilerin son yakınlaştırmasını yeniden yükle, her alanın varsayılan metin yakınlaştırması, santimetre ya da inç ve cetvelin adımı, üzerine yazma için Insert tuşu, Ekle > Geçerli Tarih için kendi tarih biçiminiz, canlı sayımın neyi gösterdiği, renksiz satır içi açıklamalar, noktalama işaretinden önce duran yorumlar ve dipnotlar, blok ekleme noktası); **Biçimlendirme** (yeni projelerin biçimlendirmesi ve stilleri); **Revizyonlar** (Biçim > Revizyon Modu için beş renk, ortak paletten).
- **Davranışlar**: **Yazım Modu** (ekran, ana pencerenin yoldan çekilmesi, Esc); **Belge Bağlantıları** (belge bağlantılarının nerede açıldığı, geri bağlantılar); **Çift Tıklama** (mantar panoda çift tıklama bir kart yapar); **Sürükle ve Bırak** (metin belgelerinin üzerine bırakılan belgeler, diğer programlara sürüklenen metnin biçimi); **Klasörler ve Dosyalar** (klasörler mantar pano olarak açılır; alt belgeleri olan bir metin klasör gibi açılır); ses ve video **Oynatma** (Bölüm 6); **Hızlı Başvuru** (paneller projeleriyle yeniden açılır); **Return Tuşu** (Return özet düzenlemeyi bitirir; Return yeni bir kart ya da satır yapar); **Anlık Görüntüler** (anlık görüntüler alındığında bir ileti).
- **Dış Görünüm**: tema ve pencerenin on bir bölümü (Genel Arayüz, Binder, Yazım Modu, Mantar Pano, Dizin Kartları, Denetçi ve Notlar, Ana Düzenleyici, Anahat, Birleşik Metin, Hedef İlerleme Çubukları, Metin İşaretleri), her biri kendi seçenekleri, yazı tipleri ve renkleriyle, ayrıca **Standart Renkler** (0.95'ten beri: yeni projelerin başladığı altı etiket, metin rengi paletinin ve vurgu paletinin altışar rengi ve iki düğmenin başladığı renkler; **Varsayılanlar** programın standardını geri verir; var olan projeler etiketlerini korur). Aşağıdaki "Temalar, renkler ve yazı tipleri" bölümüne bakın. **Genel Arayüz** ayrıca başlık çubuğunda projenin tam yolunu ve araç çubuğu simgelerinin boyutunu içerir.
- **Düzeltmeler**: yazarken düzeltmeler (Bölüm 4) ve **Yazım Denetimi** (sözlük, sayılı sözcükler, altı çizgisi, kişisel sözcük listesi).
- **Klavye**: her komutun kısayolu (Bölüm 19).
- **Paylaşım**: **İçe Aktar** (yorumlar, dipnotlar, stiller, düz metnin kodlaması, HTML dosyaları; Bölüm 14), **Dışa Aktar** (Dosya > Dışa Aktar > Dosyalar'ın dosya biçimi ve notları), **Dönüştürme** (*.doc* dosyalarının nasıl okunduğu).
- **Yedekleme**: ne zaman yedeklenecek (açılışta, kapanışta, her Dosya > Kaydet ile), kaç yedek tutulacak (yeni bir kurulum en yeni 25 ile başlar; alan yalnızca sayıyı tutar, sözcükler çevresinde durur: "Her projenin en yeni [25] yedeğini sakla") ve nerede.
- **Ek Özellikler**, {{APP}} programının kendi eklemeleri için: **Zihin Haritası** (tahtaların seçenekleri, yeni notların biçimi ve görünümü, kendi rengi olmayan tahtaların açık ve koyu temalardaki arka planı, notlardaki ve çizgilerdeki simgelerin boyutu), **Nesneler** (yeni projelerin kendi nesne şablonlarınızı mı yoksa standart olanları mı alacağı), **Eklentiler ve Uzantılar** (kurulu derleme biçimleri ve yazım araçları, uzantı klasörleri), **Çökme Raporları** (yanıt gelmeden kaç saniye sonra bir donma raporunun yazılacağı, kaç rapor tutulacağı).
- **Konuşma** (Konuşma Yazıya Dökme eklentisiyle): model, aygıt, diller, mikrofon ve duraklamalar (bkz. Bölüm 10, *Konuşma Yazıya Dökme*). Sonraki eklentilerin yazım araçları da kendilerine ait bir bölüm getirebilir.

Programın diğer pencereleri seçenekleri bir sayfada açabilir; örneğin medya oynatıcıdan Oynatma sayfası.

Güncellemelerin ve eklentilerin kendi pencereleri vardır: **Yardım > Eklentiler ve Güncellemeler > Güncelleme veya Eklenti Kur…** ve **Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi…** (Bölüm 17).

Projeye özel ayarlar (etiketler, durum, kesim türleri, metnin görünümü, özel üst veri, özel klasörler, derleme için başlık ve yazar) **Proje > Proje Ayarları** altındadır.

### Bir proje için ayarlar

**Dosya > Seçenekler** penceresinin en üstünde **Ayarların geçtiği yer:** değiştirdiklerinizin nerede tutulacağını seçer:

- **Tüm projeler (program)**: her proje için programın ayarları. Pencere burada açılır.
- **Bu proje**: açık proje burada değiştirdiklerinizi kendi ayarı olarak tutar (klasöründeki *settings/options.ini* içinde). O proje açıkken geçerlidir; projenin ayarlamadığı her şey programın ayarlarından gelir. Pencerenin başlığı "Seçenekler: Bu Proje" der ve seçimin altındaki satır projenin kaç ayarının kendisine ait olduğunu söyler (görmek için sayıya tıklayın). **Program Ayarlarını Kullan** hepsini kaldırır; programın ayarları projede yeniden geçerli olur. Seçeneklerin dışında yapılan bir değişiklik, örneğin klavye penceresinde değiştirilen bir kısayol ya da düzenlenen bir araç çubuğu, projenin bunun için zaten kendi değeri varsa projeye, yoksa programa gider.

Birkaç ayar yalnızca programa aittir ve bir proje için gösterilmez: son projeler, son projeyi yeniden açma, masaüstünün dosya iletişim kutuları, pencerelerin ve düzenlerin yerleri, çökme raporu süreleri ve yeni projeler için görünüm. Bir projenin görünümü her zaman kendisine aittir: **Dış Görünüm** sayfası açık projenin görünümünü ya da yeni projeler için görünümü gösterir (en üstteki **Şunun görünümünü göster ve değiştir**); yukarıda **Bu proje** seçiliyken yalnızca projenin görünümünü gösterir. **Varsayılanlar** ve **Yönet > Profili İçe Aktar…** yalnızca programın ayarları üzerinde çalışır; bunlar için **Tüm projeler** seçin.

**Hangi ayarlar önce gelir** (**Genel > Başlangıç**, tüm projeler için): işaretliyse (standart), bir projenin kendi ayarları önce gelir ve program geri kalanını doldurur. İşaretsizse, programın ayarları her zaman geçerlidir: projelerin kendi ayarları kullanılmaz (sonrası için projelerde kalırlar) ve her proje yeni projeler için görünümü gösterir.

**Proje > Proje Ayarlarını Dosyaya Kaydet…** projenin kendi seçeneklerini (**Bu Projenin Seçenekleri** bölümü) de içerir; böylece başka bir projeye taşınabilirler.

### Temalar, renkler ve yazı tipleri

Bir tema, pencerenin her alanının renklerini ve istiyorsa yazı tiplerini belirler. **Görünüm > Temalar** altından ya da Temalar araç çubuğunun tema düğmesiyle (araç çubuğunu **Görünüm > Araç Çubukları** altında açın) birini seçin; değişiklik yeniden başlatma olmadan hemen geçerli olur. Dokuz tema yerleşiktir:

| Tema | Görünüm |
|-------|------|
| Shelin's Dark Theme | koyu pencere ve binder, siyah metinli sıcak bir kâğıt, birleşik metnin başlıkları için Cinzel (0.95'ten beri; listenin en üstünde durur ve Temaları Yönet'te silinebilir; silinirse güncellemelerden sonra da gelmez) |
| Varsayılan (Masaüstünü Izle) | masaüstünüzün renkleri (KDE renk düzeni) |
| Açık | açık gri pencere, beyaz kâğıt |
| Koyu Mod | koyu gri, koyu dizin kartlarıyla |
| Sepya | sıcak krem ve kahverengi |
| Arduvaz | gri pencere, gri bir masada beyaz kâğıt |
| Gece Yarısı | kehribar vurgulu koyu mavi |
| Okyanus | açık mavi-yeşil |
| Yüksek Karşıtlık | siyah üzerinde beyaz ve sarı |

Eklentiler daha fazla tema getirebilir ("Extra themes" eklentisinde Northern Night, Forest, Graphite, Warm Paper ve Rose Dawn bulunur).

**Tema düğmesi** (0.90.2, 0.91 ve 0.94'te değiştirildi). Temalar araç çubuğunda, başlangıçta kapalı olan (**Görünüm > Araç Çubukları > Temalar** onu gösterir; 0.94'ten önce dört düğme ve bir liste ana araç çubuğunun üçte birini alıyordu) tek bir düğme, **Tema**. Menüsü şunları listeler:

- **Açık** ve **Koyu**: bir tıklama o temayı olduğu gibi hemen gösterir.
- **Görünümüm 1** ve **Görünümüm 2** kendi iki görünümünüzü saklar. Boş birini seçmek (menüde *boş* yazar) şu anda gördüğünüz görünümü (üzerinde değiştirdiğiniz her renk ve yazı tipiyle tema) ona kaydeder; dolu birini seçmek görünümünü geri getirir. Menünün sonundaki **Geçerli Görünümü Görünümüm 1 Olarak Kaydet** ve **...Görünümüm 2 Olarak Kaydet**, şu anda gördüğünüz görünümü oraya kaydeder (bir görünümün yerine geçmeden önce sorarlar). İki görünüm sıradan temalardır: aşağıdaki listede ve **Görünüm > Temalar** altında bulunurlar; orada dışa aktarılabilir ya da silinebilirler (silinen bir girdisini boş bırakır).
- **Tüm temalar**, aşağıda, kullanımdaki işaretli olarak. Birini seçmek **Görünüm > Temalar**'ın yaptığını yapar: açık proje ve yeni projeler onu kullanır ve kendi değişikliklerinizi üstte tutup tutmama sorusu eskisi gibi gelir.
- **Kendi görünümünüz bir seçimle asla kaybolmaz.** Menü bir temayı olduğu gibi gösterir; böylece gördüğünüz gerçekten değişir. Gösterilen temanın üstünde kendi renkleriniz ya da yazı tipleriniz varsa bununla kaybolurlardı; bu yüzden önce kaydedilirler: boş bir Görünümüm'e soru sorulmadan (durum çubuğu nereye olduğunu söyler) ve ikisi de bir görünüm tutuyorsa {{APP}} Görünümüm 1'e mi, Görünümüm 2'ye mi kaydedeceğini ya da kaydetmeden geçip geçmeyeceğini sorar.

Her araç çubuğu denetimi gibi, tema düğmesi **Görünüm > Araç Çubukları > Araç Çubuklarını Özelleştir** altında taşınabilir ya da kaldırılabilir ve Temalar araç çubuğunun tamamı **Görünüm > Araç Çubukları** altında açılıp kapatılabilir.

**Koyu temalarda sayfa** (0.70). {{APP}} programının standart sayfası, gri bir masanın yanında açık zeytin renginde bir kâğıttır. Koyu bir temayla sayfa bunun yerine beyaz metinli çok koyu gridir (ya da varsa temanın kendi koyu sayfası); böylece koyu bir tema koyu bir çerçeve içinde parlak bir sayfa bırakmaz. Kendiniz seçtiğiniz bir sayfa rengi (sağ tık > Dış Görünüm ya da Dosya > Seçenekler > Dış Görünüm) her temada kalır.

**Renkleri ve yazı tiplerini değiştirmek.** **Dosya > Seçenekler > Dış Görünüm** (ya da **Görünüm > Temalar > Renkler ve Yazı Tipleri…**) solda pencerenin bölümlerini listeler. Her biri seçeneklerini, yazı tiplerini ve renklerini gösterir (bir renge tıklamak ortak paleti gösterir: etiket renkleri, altı standart renk ve **Diğer...**):

| Bölüm | Yazı tipleri | Renkler |
|------|-------|--------|
| Genel Arayüz | menüler, düğmeler ve paneller | pencere, Denetçi Arka Planı (denetçinin listeleri ve metin alanları), değişen satırlar, metin, düğmeler, seçim, bağlantılar, ipuçları, kenarlıklar, gri metin |
| Binder | Binder | arka plan, başlıklar |
| Yazım Modu | (yok) | arka plan, kâğıt, metin |
| Mantar Pano | (yok) | mantar pano, durum damgası |
| Dizin Kartları | kart başlıkları, kart metni | dizin kartı, başlık, metin, başlığın altındaki çizgi, çizgili satırlar |
| Denetçi ve Notlar | özet | Denetçi Şeritleri (sekme sırası, başlıklar, Etiket ve Durum satırı), etiketler, özet kartı ve metni, notlar ve metinleri |
| Ana Düzenleyici | üst bilgi çubuğu başlığı | kâğıt, metin, metin sütununun yanındaki boşluk, sayfaların çevresindeki masa (yalnızca sayfa görünümünde gösterilir), üst bilgi çubuğu (normal, etkin düzenleyici, yerine kilitli) |
| Anahat | anahat | arka plan, değişen satırlar, metin |
| Birleşik Metin | birleşik metindeki başlıklar | (yok) |
| Hedef İlerleme Çubukları | (yok) | oturum, taslak, hedefe ulaşıldı |
| Metin İşaretleri | (yok) | metindeki bağlantılar, arama sonuçları, yazım altı çizgisi, Görünmeyen Karakterleri Göster'in işaretleri |

**Her proje kendi görünümünü korur.** Binder'ın, düzenleyicinin, mantar panonun, anahatın, denetçinin ve yazım modunun teması, bunlar için kendi renkleriniz ve yazı tipleriniz, metin sütununun genişliği ve metnin yanındaki kenarlık projeye aittir: bunları bir projede değiştirmek diğerlerini olduğu gibi bırakır ve {{APP}} programının bir güncellemesi mevcut bir projenin görünümünü asla değiştirmez. **Dosya > Seçenekler > Dış Görünüm** en üstündeki **Şunun görünümünü göster ve değiştir** listesi, sayfanın kimin görünümünü gösterip değiştireceğini seçer: **Bu proje** (açık olan; ilk seçilen) ya da **Yeni projeler** (yeni bir projenin başladığı görünüm). İkisi arasında geçiş yapmak, siz **Uygula** ya da **Tamam** düğmesine basana kadar her birinde değiştirdiklerinizi korur. Menülerin ve panellerin renkleri (**Genel Arayüz**) ile menülerin, düğmelerin ve panellerin yazı tipi her projede aynıdır. 0.40.2 ya da sonraki sürümle ilk kez açılan bir proje o ana dek sahip olduğu görünümü korur. **Görünüm > Temalar** açık projenin ve yeni projelerin temasını aynı anda belirler.

**Sağ tık menüleri.** Nereye sağ tıklarsanız tıklayın, menüde fare imlecinin altındakinin renkleri ve yazı tipleriyle bir **Dış Görünüm** alt menüsü vardır: metin ve yanındaki kâğıt üzerinde kâğıt, metin, sütunun yanındaki boşluk, bağlantılar, arama sonuçları, yazım altı çizgisi, sütun genişliği ve kenarlık; bir düzenleyicinin üstündeki üst bilgi çubuğunda renkleri ve yazı tipi; Binder'da arka planı, başlıkları, yazı tipi, girintisi ve etiket renkleri; mantar panoda mantar, dizin kartları ve yazı tipleri, durum damgaları, kart genişliği; anahatta arka planı, satırları, metni ve yazı tipi; denetçide ve notlarında şeritler, etiketler, özet kartı ve notlar ile her projede aynı olan Denetçi Arka Planı; yazım modunda arka planı, kâğıdı ve metni. Bir renk ortak paleti ve **Temanın Rengi**'ni gösterir; **Dış Görünüm Seçenekleri...** seçeneklerin bu sayfasını açar. Seçenekler gibi, bu menüler açık projenin görünümünü (alt menünün ilk satırı bunu söyler) ya da proje yoksa yeni projelerin görünümünü değiştirir.

Değiştirmek için bir renge, yazı tipi seçmek için bir yazı tipi düğmesine tıklayın. Üstteki küçük resim, **Uygula** ya da **Tamam** düğmesine basmadan önce sonucu gösterir. Değişiklikleriniz temanın üstünde durur: değiştirilmiş bir girdi kalın gösterilir, yanındaki **↺** düğmesi onu temaya geri verir ve **Renkleri ve Yazı Tiplerini Temaya Sıfırla** hepsi için bunu yapar. Değiştirmediğiniz ve temanın ayarlamadığı bir renk diğerlerini izler; örneğin Binder, kendine ait bir renk verene kadar listelerin rengini kullanır. **Denetçi Arka Planı** yalnızca denetçinin listelerini ve metin alanlarını renklendirir; araç çubuklarındaki alanlar, cetvel ve seçeneklerin listeleri pencerenin ve menülerin rengini taşır.

Yazdığınız metnin yazı tipi ve boyutu temanın parçası değildir: projeye aittir (**Proje > Proje Ayarları > Biçimlendirme**, Bölüm 4).

**Kendi temalarınız.** Seçeneklerdeki **Tema Olarak Kaydet…** (ya da **Görünüm > Temalar > Geçerli Görünümü Tema Olarak Kaydet…**) temayı değişiklikleriniz ile birlikte yeni bir adla saklar. Bundan sonra diğerleri gibi **Görünüm > Temalar** içinde görünür. **Görünüm > Temalar > Temaları Yönet…** tüm temaları listeler: **Bu Temayı Kullan**, **Çoğalt…** (kendi başlangıç noktanızı almak için yerleşik temalar için de), **Yeniden Adlandır…** ve **Sil** (yalnızca kendi temalarınız), **Dışa Aktar…** (iletebileceğiniz bir *.qbtheme* dosyası yazar) ve **İçe Aktar…** (böyle bir dosyayı ekler).

Kendi değişiklikleriniz varken başka bir tema seçtiğinizde, {{APP}} bunları yeni temayla birlikte tutup tutmayacağınızı ya da temayı olduğu gibi kullanıp kullanmayacağınızı sorar.

Metninizdeki bağlantılar tek bir mavi ile saklanır; koyu kâğıtta daha açık bir mavi ile gösterilirler (**Ana Düzenleyici > Metindeki bağlantılar**). Kendiniz renklendirdiğiniz metin rengini her temada korur.

### Profiller

Bir tema yalnızca bir görünümdür: renkler ve yazı tipleri. Bir **profil** (0.95'ten beri) programın ayarlarının geri kalan her şeyini tutar: dili, tüm seçenekleri, araç çubuklarını ve yerlerini, pencerenin boyutunu ve yerini, gösterilen panelleri, ayırıcıları, klavye takımını ve kısayollarınızı, yedekleri ve yedek klasörünü, güncelleme denetimlerini, standart renkleri, **Görünüm > Yerleşim** altındaki kayıtlı yerleşimleri ve temasını adlandırır; bu yüzden profili değiştirmek görünümü de değiştirir. Adınız ve adresiniz, bu bilgisayarın adı, son projeler ve karşılama projesinin yerleri bir profilin parçası değildir.

- **Değiştirme.** **Görünüm > Profiller** profilleri listeler; bir tıklama yeniden başlatmadan hemen geçer (yeniden başlatma gerektiren tek şey dildir; program bunu söyler). Ayrılacağınız profil önce kaydedilir. **Profiller** araç çubuğu (**Görünüm > Araç Çubukları > Profiller**, başlangıçta kapalı) kullanımdaki profili gösterir; düğmesine tıklamak profilleri listeler. Bir projenin kendi ayarları (**Ayarların geçtiği yer: Bu proje**) yine profilin üstüne gelir. Açık proje, **Görünüm > Temalar** altında bir tema seçtiğinizde olduğu gibi, yeni profilin görünümünü gösterir.
- **Default.** Daha önce kullanılmış bir program, 0.95'e güncellendiğinde ayarlarını **Default** profili olarak alır, böylece hiçbir şey kaybolmaz. Yeni bir kurulum programın standart ayarlarıyla başlar; sistem bildiriyorsa, sisteminizin ayarına göre açık ya da koyu temayla. İlk başlangıç sayfası **Default** profilini doldurur. **Default** silinemez.
- **Yeni bir profil**, ayarların şu andaki hâlinden: **Görünüm > Profiller > Ayarları Yeni Profil Olarak Kaydet…** (ya da **Profilleri Yönet… > Ayarları Yeni Profil Olarak Kaydet…**). Yeni profil kullanımdaki profil olur. **Profilleri Yönet…** ayrıca profilleri yeniden adlandırır, kopyalar ve siler (kullanımdaki profil silinemez; önce başkasına geçin).
- **Shelin's**, programla birlikte gelen bir profildir. Listenin en üstünde durur, yeni metin için Sitka'yı ve kendi temasını ayarlar ve silinebilir; silinirse güncellemelerden sonra da gelmez. Adını verdiği ve kurulu olmayan yazı tipleri (Cinzel, Sitka) başkalarına döner. Oynatma motoru seçimi Extra Video and Audio Formats eklentisini gerektirir; yazım araçlarının model seçimleri yalnızca o eklentilerin ve modellerin kurulu olduğu yerde geçerlidir.
- **Başka bir ekranda boyutlar.** Binder'ın, düzenleyicinin ve denetçinin genişlikleri pencerenin payları olarak saklanır; büyük bir ekranda yapılmış bir profil daha küçük bir ekranda ya da tablette aynı payları gösterir (yan sütunlar okunabilir bir genişlik korur) ve pencere ekrana getirilir.
- **Dışa ve içe aktarma.** **Profilleri Yönet… > Dışa Aktar…** (ya da **Dosya > Seçenekler > Yönet > Profili Dışa Aktar…**) profili yerleşimleriyle ve, sizin kendi profilinizse, temasıyla tek bir dosyaya (*.qbprofile*) yazar. **İçe Aktar…** böyle bir dosyayı yeni bir profil olarak ekler (ona geçilmez). Bir dosya programları, klasörleri, dosyaları ya da adresleri adlandırabilir; başkasından gelen diğer dosyalarda olduğu gibi, bunlar alınmadan önce size sorulur (bu bilgisayarda bulunmayan bir klasör işe yaramaz). Eski bir seçenekler dosyası (*.qboptions*) de profil olarak içe aktarılabilir.

**Proje > Proje Ayarlarını Dosyaya Kaydet…** olduğu gibi kalır.

---

## 17. Güncellemeler ve eklentiler

**Yardım > Sorun Giderme ve Geliştirme > Geliştirici Kılavuzu: Güncellemeler ve Eklentiler**, {{APP}} programını derleyen ya da eklenti yazan kişiler için güncellemelerin ve eklentilerin nasıl çalıştığını ve hangi araçların onları yaptığını açıklar; ayrıca teknik bilgisi olan bir arkadaşın işleri kurmak için ihtiyaç duyacağı şeyleri (medya motorları, derleme biçimi uzantıları, paket, tema, düzen ve senaryo biçimi dosyaları, bir proje klasörünün içindeki dosyalar) ve programın okuduğu ortam değişkenlerini içerir.

{{APP}} tek bir program dosyasıdır (AppImage). Güncellemeler ve uzantılar **.qbpkg** ile biten küçük paket dosyaları olarak gelir. Üç tür vardır ve hepsi aynı şekilde kurulur:

- **Program güncellemesi**: {{APP}} programının kendisinin daha yeni bir sürümü.
- **Eklenti**: açıp kapatabileceğiniz bir uzantı; örneğin renk temaları, proje şablonları, derleme biçimleri, sözlükler, yazım araçları (Bölüm 10), ses ve video oynatma ve ek biçimleri (Bölüm 6), eski Word dosyaları ve web sayfaları için içe aktarıcılar (Bölüm 14).

Program güncellemeleri ve eklentiler, AppImage'ın yanında, proje sayfasının (Bölüm 1) **Sürümler** (Releases) sayfasındadır.
- **Güncelleme paketi**: bir program güncellemesini ve birkaç eklentiyi içeren ve birlikte kurulan tek bir dosya.

### Güncellemeleri denetlemek

0.92.0'dan beri {{APP}} güncellemeleri kendisi arayabilir. Programın kendiliğinden internete bağlandığı tek şeydir ve bunu kapatabilirsiniz.

- **Yardım > Eklentiler ve Güncellemeler > Başlangıçta Güncellemeleri Denetle** anahtardır (başlangıçta açık). **Yardım > Eklentiler ve Güncellemeler > Güncellemeleri Şimdi Denetle** anahtar ne derse desin hemen bakar.
- **İlk başlangıç** bunu, hiçbir yere bağlanılmadan önce, kendi sayfasında söyler: hakkınızda hiçbir şeyin gönderilmediği notuyla bir *Program başlarken güncellemeleri ara* işaret kutusu (işaretli); bir **Tamam** sayfayı bitirir. 0.94.0'dan önce başlatılmış ve notu görmemiş bir program, bunu **Tamam** ve **Kapat** düğmeleriyle bir kutu olarak gösterir.
- **Ne olur**: başlangıçtan birkaç saniye sonra, günde en çok bir kez, program proje sayfasından küçük bir dosya (*shelin-boop.github.io/Quillbinder_Collection/updates-1.json*) ve o dosyanın imzasını getirir. İstek programı ("{{APP}}") adlandırır ve başka hiçbir şey yok: sürüm, sistem, tanımlayıcı, çerez yok. Her bağlantıda olduğu gibi sunucu (GitHub) internet adresinizi ve saati görür. Dosya programın ve projenin eklentilerinin en yeni sürümünü listeler; programınız bunu kurulu olanla bilgisayarınızda karşılaştırır. Dosya yalnızca projenin yayın anahtarının imzasını taşıyorsa geçerlidir ve daha önce görülen bir dosyadan eski bir dosya yok sayılır.
- **Bir güncelleme olduğunda**, bir pencere onu adlandırır: program ya da eklenti, sürümünüz ve yeni sürüm, yenilikler ve türü. **İşlevsel** bir güncellemenin üç düğmesi vardır: **İndir ve Kur**, **Bu Sürümü Yok Say** (o sürüm için artık hatırlatma yok) ve **Ertele**. Ertele her seferinde daha uzun bekler: 1 gün, 1 hafta, 2 hafta, 4 hafta; sonra son bir hatırlatma gelir ve ondan sonra o sürüm için hatırlatmalar biter. **Önerilen (güvenlik)** bir güncellemenin yalnızca iki düğmesi vardır, **İndir ve Kur** ve **Ertele**: yok sayılamaz ve dördüncü ertelemeden sonra her 4 haftada bir geri gelir. İkisi de olduğunda tek bir pencere onları listeler ve güvenlik kurallarını izler. Pencere işinizin yanında gelir ve klavyeyi almaz.
- **İndir ve Kur**, listenin adlandırdığı dosyaları getirir, her birini imzalı listedeki boyut ve sağlama toplamıyla denetler ve kurar: bir eklentiyi ya da program güncellemesini **Yardım > Eklentiler ve Güncellemeler > Güncelleme veya Eklenti Kur** gibi (yalnızca projenin imzasını taşıyan paketler). Sonra programı yeniden başlatmayı önerir.
- **Yeni sürüm yeni bir AppImage gerektirdiğinde** (AppImage'ınızdakinden başka kitaplıklar), {{APP}} onu indirir, başlattığınız AppImage'ın yerine, nerede olursa olsun ve aynı dosya adıyla koyar, elinizdekini yanında *…AppImage.before-update* olarak tutar ve yeniden başlar. Yeni sürüm sizin için çalıştığında tutulan dosyayı silin. AppImage'ınızın klasörü yazılamıyorsa, yeni dosya İndirilenler klasörünüze gider ve bir ileti ne yapılacağını söyler.
- **Windows'ta** (0.92.0) böyle bir sürüm kendi kurulum programı olarak gelir. {{APP}} onu kendi klasörüne (*data\update-setup*) indirir, projenin imzalı listesiyle denetler ve sorar. **Kapat ve Şimdi Güncelle** ile {{APP}} kapanır ve kurulum programı yeni sürümü sayfaları olmadan kullandığınız klasöre açar ve başlatır; ayarlarınız, eklentileriniz ve *data* içindeki her şey kalır. **Sonra** ile hiçbir şey değişmez ve güncelleme yeniden önerilir. Windows kurulum programını başlatmazsa (kod imzalama sertifikası taşımaz), bir ileti dosyayı adlandırır ve güncel sürümü proje sayfasından da indirebilirsiniz.
- **Adrese ulaşılamadığında**, program 5 saniye arayla iki kez dener ve sonra ilk düzenleyicinin alt bilgi çubuğunda, kayıt metninin yerinde, iki kez belirip kaybolan küçük bir metin gösterir. Pencere yok ve günde en çok bir kez. (**Güncellemeleri Şimdi Denetle** bunun yerine bir iletiyle yanıt verir.)
- **Kapalıyken**, program 0.92.0'dan önceki gibi hiçbir yere bağlanmaz. Başkaları için kurulan bilgisayarlarda, `QB_NO_UPDATE_CHECK=1` ortam değişkeni, anahtar ne derse desin otomatik denetimin çalışmasını engeller.
- 0.92.0'dan önceki bir sürümün AppImage'ı, daha yeni bir program güncellemesi çalıştırıyorsa güvenli bağlantılar kuramaz; orada denetim siz istediğinizde bunu söyler ve aksi halde sessiz kalır. Güncel AppImage'ı bir kez indirin.
- Programın bu konuda hatırladıkları (anahtar, son bakışın zamanı, yok saydıklarınız ya da ertelediğiniz) programın veri klasöründeki *updates-state.json* içindedir.

### Bir paketi kurmak

1. **Yardım > Eklentiler ve Güncellemeler > Güncelleme veya Eklenti Kur…** komutunu seçin ve .qbpkg dosyasını seçin ya da dosyaya dosya yöneticinizde çift tıklayın.
2. {{APP}} paketin neleri içerdiğini ve nelerin yeni olduğunu gösterir. **Kur** düğmesine tıklayın.
3. Paket bir program güncellemesi içeriyorsa, {{APP}} yeniden başlatma ister. Açık projeniz kaydedilir ve yeniden açılır.

{{APP}} her paketi kurmadan önce denetler: bozuk dosyalar ve başka bir sürüm için yapılmış paketler reddedilir. Bir program güncellemesi bir kez görünmez olarak deneme amaçlı başlatılır; bilgisayarınızda çalışmazsa kurulmaz. Bir paketin parçaları birlikte kurulur: biri kurulamazsa hiçbiri kurulmaz (0.65).

**İmzalar** (0.65). Projenin sürüm sayfasındaki her paket projenin anahtarıyla imzalanır ve {{APP}} bu anahtarın genel yarısını taşır. Kurulum penceresi böyle bir paket için **Quillbinder projesi tarafından imzalandı** der: bu, projenin yayımladığı, değiştirilmemiş pakettir.

- İmzası uyuşmayan bir paket (imzalandıktan sonra değiştirilmiş ya da bozulmuş) asla kurulmaz.
- Bir **program güncellemesi** yalnızca projenin imzasıyla kurulur.
- İmzası olmayan bir **eklenti** (sizin ya da başka birinin yaptığı) yalnızca açık bir uyarıdan sonra, varsayılan olarak **İptal** ile kurulur. Komut satırında (`--install <dosya>`), projenin imzasını taşımayan bir paket yalnızca `--allow-unsigned` eklediğinizde kurulur.

**Eklenti anahtarları** (0.91.3). Resmi eklentiler, başka yazarlarınkiler de dahil, projenin anahtarı yerine kendi anahtarlarıyla imzalanabilir. Proje böyle bir anahtarın arkasında, paketin içinde gelen ve anahtarın eklentileri imzalamasına, başka hiçbir şeye izin veren bir sertifikayla kendi anahtarıyla durur. Kurulum penceresi bu durumda anahtarı adlandırır: *Signed with the add-on key "Quillbinder official add-ons", which the Quillbinder project vouches for*.

- Bir eklenti anahtarıyla imzalanmış bir **program güncellemesi** imzasız gibi reddedilir: program güncellemeleri projenin kendi anahtarını gerektirir.
- Projenin arkasında durmadığı bir eklenti anahtarı imzasız sayılır: eklenti yalnızca uyarıdan sonra kurulur.
- 0.91.2'ye kadar hepsinin olduğu gibi, projenin kendi anahtarıyla imzalanmış eklentiler güvenilir kalır. 0.91.3'ten itibaren projenin kendi eklentileri de eklenti anahtarıyla imzalanır.
- 0.91.2'ye kadar olan sürümler eklenti anahtarlarını bilmez ve bir tanesiyle imzalanmış bir eklentiyi kurmadan önce uyarır: önce program güncellemesini kurun (projenin kendi anahtarıyla imzalanmıştır), sonra eklentileri.

**Bir eklenti kurulmadan önce soran pencere** (0.94.0) önce bir iki cümleyle eklentinin ne yaptığını söyler. Teknik satırlar (getirdiği kitaplıklar, konuşma ya da grafik motoru, için yapıldığı çalışma zamanı) siz açana kadar katlı olan **Ayrıntılar** altındadır.

**Bir indirmeyi kendiniz denetlemek.** Her sürüm ayrıca *SHA256SUMS.txt* (her dosyanın sağlama toplamı), imzası *SHA256SUMS.txt.sig* ve genel anahtar *quillbinder-signing-public.pem* dosyalarını içerir. Bir terminalde, indirmelerin bulunduğu klasörde:

    openssl pkeyutl -verify -pubin -inkey quillbinder-signing-public.pem -rawin -in SHA256SUMS.txt -sigfile SHA256SUMS.txt.sig
    sha256sum -c --ignore-missing SHA256SUMS.txt

Birinci komut *Signature Verified Successfully*, ikincisi indirdiğiniz her dosya için *OK* yanıtını verir. Anahtar dosyasının kendisinin projeninki olduğundan emin olmak için, 32 baytını (`openssl pkey -pubin -in quillbinder-signing-public.pem -outform DER | tail -c 32 | od -An -tx1`) programın kaynağındaki anahtarla (*src/core/Signing.cpp* ve *packaging/quillbinder-signing-public.pem*) karşılaştırın: *bc 3c 58 00 08 3f 66 e6* ile başlarlar.

### Eklenti Yöneticisi

**Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi…** nelerin kurulu olduğunu gösterir.

- **Program sürümü**: AppImage'ınıza yerleşik sürüm ve kurulu her güncelleme. *şu anda çalışıyor* işaretli olan kullanımdadır. Daha eski bir sürüme dönmek için onu seçin ve **Bu Sürümü Kullan** düğmesine tıklayın, sonra yeniden başlatın. Artık gerek duymadığınız kurulu güncellemeler kaldırılabilir.
- **Eklentiler**: bir eklentiyi kaldırmadan kapatmak için işaretini kaldırın; **Eklentiyi Kaldır** onu siler. Bir eklentinin temaları, şablonları ve derleme biçimleri olağan yerlerde görünür: **Görünüm > Temalar**, **Yeni Proje** penceresi ve **Derle** penceresi.
- Burada yapılan bir seçim yazılamadığında (dolu bir disk, yazılamayan bir klasör), bir ileti bunu dosyanın adıyla söyler (0.92.0). Önceden pencere seçimi yapılmış gösteriyordu ve bir sonraki başlangıçta tek söz söylenmeden geri alınıyordu. Aynısı bir kurulum için de geçerlidir: girdisi yazılamayan bir eklenti ya da program sürümü, ne yapılacağıyla birlikte kurulmamış olarak bildirilir. Bir kurulum yarıda başarısız olduğunda ve önceden kurulu sürüm geri konamadığında, ileti bulunduğu klasörü ve o klasöre verilecek adı adlandırır.

### Bir güncelleme başlamazsa

Bir program güncellemesi art arda iki kez başlamazsa, {{APP}} onu kendiliğinden kapatır ve AppImage'a yerleşik sürümü, bunu söyleyen bir iletiyle başlatır. Projelerinizde hiçbir şey bundan etkilenmez.

### Yeni bir AppImage gerektiğinde

Program güncellemeleri AppImage'ınızın içindeki kitaplıklar üzerinde çalışır. Zaman zaman daha büyük bir güncelleme yeni bir AppImage gerektirir; kurulum penceresi bir paket sizinkine uymadığında bunu söyler. Eski AppImage dosyasını yenisiyle değiştirin; ayarlarınız, eklentileriniz ve projeleriniz olduğu gibi kalır.

0.80.0 böyle bir sürümdür: AppImage'ı yeni bir Qt taşır (Ubuntu'nun Qt 6.4.2'si yerine The Qt Company'nin kendi Qt 6.12 yapısı). Program kodlu eklentiler (*Media Playback*, *Extra Video and Audio Formats*, *Speech Transcription*, yazım araçları ve içe aktarıcılar) 0.80.0 için olan sürümleriyle yeniden kurulmalıdır (Sürümler sayfasındaki paket ve **media-all-formats.qbpkg** hepsini getirir); o zamana kadar Eklenti Yöneticisi eskileri daha eski bir program için yapılmış olarak işaretler ve kodları yüklenmez. 0.80.0'ın tam sürümü her eklentiyi getirir.

0.93.0 bir başkasıdır (Linux): AppImage'ı, yerleşik kitaplıkları (zlib, HarfBuzz, PCRE2, libpng, libjpeg-turbo, FreeType ve diğerleri) daha yeni olan ve güvenlik düzeltmeleri içeren Qt 6.12.1 taşır. Bir program güncellemesi bunları getiremez; bu yüzden güncelleme denetimi yeni AppImage'ı önerir. Önceki AppImage'ın program kodlu eklentileri (yukarıda listelenenler) aynı şekilde 0.93.0 için olan sürümleriyle yeniden kurulur ve medya eklentileri artık FFmpeg 9.1 taşır.

Yeni bir AppImage ayrıca kendi yeni program sürümünü de getirir (örneğin AppImage 0.20.0). Eski AppImage'a güncellemeler kurmuş olsanız bile o sürümü başlatır: eski güncellemeler kapatılır ve **Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi…** altında listelenmiş kalır; orada kaldırabilirsiniz. Yeni dosyanın adı eskisinden farklıysa ya da başka bir klasördeyse, menü girdisinin ve çift tıklanan projelerin yeni dosyayı kullanması için bir kez **Yardım > Uygulama Menüsüne Ekle** komutunu seçin.

---

## 18. Markdown lehçesi

{{APP}} programını kullanmak için bunu bilmeniz gerekmez; düzenleyici biçimlendirilmiş metni gösterir. Yalnızca metin dosyalarını başka bir programla açarsanız önemlidir. {{APP}} Markdown'ı şu kurallarla yazar:

- **Her satır bir paragraftır**, bir metin işlemcisinde olduğu gibi. Boş bir satır boş bir paragraftır.
- Başlıklar: satırın başında `# ` ile `###### ` arası.
- Blok alıntı: `> `. Ortalanmış paragraf: `->text<-`. Diğer her paragraf stili: `::Style Name:: text`.
- Karakter stilleri (0.4 sürümünden itibaren): `<span class="Style Name">text</span>`. Metne yalnızca stillerin adları yazılır. Bir stilin neye benzediği projenin dosyasında bir kez saklanır; bu yüzden değiştirilen bir stil belgelerde değişiklik gerektirmez.
- Listeler: `- ` ve `1. `; iç içe her düzey için iki boşluk.
- `**bold**`, `*italic*`, `~~strikethrough~~`, `==highlight==`, `<u>underline</u>`, `<sup>superscript</sup>`, `<sub>subscript</sub>`.
- Revizyon renkleri: `<rev1>…</rev1>` ile `<rev5>…</rev5>` arası.
- Bağlantılar: `[text](https://example.com)`; projedeki belgelere bağlantılar: `[text](qb://ID)`.
- Resimler: `![](images/picture.png){width=300}` (resimler belgenin klasöründe saklanır). Bir resmin açıklaması (0.62), köşeli parantezlerin içindeki metindir: `![A brass lamp](images/lamp.png){width=300}`.
- Tablolar (0.62): tasarımı küme parantezleri içinde (JSON) olan bir `:::table` satırı, hücreleri `|` arasında olan her satır için bir satır (bir hücrenin paragrafları `<p/>` ile ayrılır, birleştirilmiş bir hücrenin kapladığı hücre boş bırakılır, metindeki bir `|` `\|` olarak yazılır) ve sonda bir `:::` satırı.
- Metnin yanında ya da ortalanmış bir resim (0.62): `:::figure {"pos":1,"width":168}` (1 sol, 2 sağ, 3 ortalanmış), resmin satırı, resim yazısının satırları ve `:::`.
- Bir paragraf içinde satır sonu: `<br>`.
- El ile ayarlanan yazı tipleri (0.3 sürümünden itibaren): `<span style="font-family:'DejaVu Sans'; font-size:14pt; color:#aa0000">text</span>`. Bildirimler sıradan CSS'tir; bu yüzden HTML'ye izin veren diğer Markdown programları da bunları gösterir. Olası girdiler: `font-family`, `font-size` (pt), `font-weight` (100 ile 900 arası), `color`, `letter-spacing` (em), `font-variant:small-caps`, `text-transform:uppercase`.
- El ile ayarlanan paragraf biçimlendirmesi (0.3 sürümünden itibaren): satırın başında, diğer her ön ekten önce küme parantezleri içinde bir liste: `{: align=right left=10mm right=5mm first=0mm before=6pt after=12pt line=1.5 tabs=20mm,50mmR} text`. `line` bir katıdır (`1.5`), kesin bir yükseklik (`14pt`) ya da en az bir değerdir (`14pt+`). Sekme durakları `R` (sağ), `C` (ortalanmış) ya da `D` (ondalık nokta) ile biter; harf yoksa sol demektir. Yalnızca ayarladığınız girdiler yazılır.
- Senaryo öğeleri (0.10 sürümünden itibaren) öğenin adını taşıyan paragraf stilleridir: `::Script > Scene Heading:: INT. LIGHTHOUSE - NIGHT`. Bir öğenin neye benzediği projenin dosyasında, senaryo biçimiyle birlikte bir kez saklanır.
- Bir paragraftan önce sayfa sonu (içe aktarılan senaryolardan ve RTF dosyalarından, 0.10 sürümünden itibaren): `{: page=before} text`.
- 0.62'den önceki sürümler tabloları ve şekilleri metin satırları olarak gösterir (`:::table` ...). Daha eski bir sürüme dönerseniz, tablolu ya da metnin yanında resimli belgeleri düzenlemeyin.
- 0.2 sürümü her iki işareti de düz metin olarak gösterir. Eklenti Yöneticisi ile 0.2 sürümüne dönerseniz, bunları içeren belgeleri düzenlemeyin.
- 0.4'ten önceki sürümler kendi stillerinizi bilmez: `<span class=…>` öğesini düz metin olarak gösterir ve kendi stillerinizdeki paragrafları gövde metni sayar. Daha eski bir sürüme dönerseniz, kendi stillerinizi kullanan belgeleri düzenlemeyin.
- 0.10'dan önceki sürümler senaryo öğelerini gövde metni sayar ve bir senaryoyu düzenleyicide gösterilip proje kaydedilir kaydedilmez öğelerin adlarını ondan kaldırır. Daha eski bir sürüme dönerseniz, senaryolarınızı orada açmayın (bkz. Bölüm 20).
- Ters eğik çizgi, sonraki karakteri sıradan metin yapar: `\*` bir yıldızdır, italik değil.
- Notlar CriticMarkup kuralını izler:
  - `{>>inline annotation<<}`
  - `^[inline footnote]`
  - `{==text==}{>>linked comment<<}`
  - `{==text==}^[linked footnote]`
  - Bir yorum bir renkle başlayabilir: `{>>@#ff8800 text<<}`.

**Dosya > Markdown ⇄ RTF Dosyasını Dönüştür…** hem bu lehçeyi hem de standart Markdown'ı (paragrafların boş satırlarla ayrıldığı) okur ve Markdown yazarken ikisinden hangisini istediğinizi sorar.

---

## 19. Klavye kısayolları

### Kısayolları değiştirmek

**Dosya > Seçenekler > Klavye** (ya da **Yardım > Klavye Kısayolları**) her komutu menüye göre gruplayarak, kısayolu ve programın birlikte geldiği standart kısayolla listeler. Açık projenin stilleri ve düzenler de listelenir; böylece bir stil ya da düzen kendi kısayoluna sahip olabilir.

1. Komutu bulun: kaydırın ya da arama alanına adının bir bölümünü ya da "Ctrl+K" gibi bir kısayolu yazın.
2. Komuta tıklayın, listenin altındaki kısayol alanına tıklayın ve yeni tuşlara basın.
3. Başka bir komutta o kısayol varsa, onu oradan alıp almayacağınız sorulur. Diğer komut, ona yeni bir tane verene kadar kısayolsuz kalır.

**Kısayol Yok** seçili komutun kısayolunu kaldırır, **Standart** birlikte geldiğini geri verir ve **Tümü Standarda** tüm değişikliklerinizi geri alır. Değişiklikleriniz kalın gösterilir. Bir kısayol Ctrl, Alt ya da Meta gerektirir ya da bir işlev tuşu olmalıdır; çünkü düz harfler yazmak için gereklidir. **Dışa Aktar** ve **İçe Aktar** kısayol kümenizi bir dosya (*.qbkeys*) olarak kaydeder ve birini yükler; örneğin başka bir bilgisayara taşımak için.

Stillerin kısayolları stilin adına aittir: "Block Quote" bu adda bir stili olan her projede kendi kısayoluna sahiptir.

### Kısayollar ve klavye düzeni

Standart kısayollardan birkaçı, bir Alman klavyesinde **AltGr** gerektiren tuşlardır: **Ctrl+{**, **Ctrl+|** ve **Ctrl+}** (sola, ortaya, sağa hizala), **Ctrl+[** ve **Ctrl+]** (geri ve ileri), **Ctrl+\\** (senaryo öğeleri listesi) ve **Ctrl+=** (alt simge). 0.94.0'dan beri program sistemin klavye düzenine **ilk başlangıçta** (hiçbir kısayol ayarı ve önceki bir başlangıç yokken) bakar ve yanıtı saklar. Bir **ABD düzeniyle** ya da Almancadan başka herhangi bir düzenle yukarıdaki olağan küme kalır. Bir **Alman (ya da Avusturya) düzeniyle** bu komutlar AltGr gerektirmeyen tuşlar alır (aynı takım, ayraçları ve küme ayraçları da AltGr gerektiren bir **Fransız, Belçika, İspanyol, İtalyan, Portekiz, Türk ya da Felemenk düzeniyle** de verilir; AltGr harfleri ć ve ń Ctrl+Alt+C ve Ctrl+Alt+N ile çakışan Polonya programcı düzeni yalnızca tablonun altında adı geçen dört komutu taşır):

| Komut | Alman kümesi |
|---------|------------|
| Sola / ortaya / sağa hizala (zihin haritasında da) | Alt+Shift+L / Alt+Shift+E / Alt+Shift+R |
| İki yana yasla (zihin haritası) | Alt+Shift+J |
| Geri / ileri | Alt+Shift+B / Alt+Shift+F |
| Senaryo öğeleri listesi | Ctrl+Shift+O |
| Alt simge | Ctrl+Shift+D |
| Öne getir / arkaya gönder (zihin haritası) | Ctrl+Shift+PageUp / Ctrl+Shift+PageDown |

AltGr'yi harfler için kullanan düzenler (Polonya, Brezilya ve yukarıdaki diğerleri) ayrıca dört komutu, Windows'un AltGr olarak okuduğu Ctrl+Alt+harf tuşlarından uzaklaştırır: biçimi kopyala Ctrl+Shift+Y, biçimi yapıştır Ctrl+Shift+U, belge notlarına git Ctrl+Shift+X ve (Polonya düzeninde değil) cetveli göster Ctrl+Shift+W. Rus ve Endonezya klavyeleri olağan kümeyi korur.

Menüler her tuşu düzenin adlandırdığı gibi gösterir. 0.94.0'dan önce kullanılmış bir program kısayollarını olduğu gibi korur (hiçbir şey kendiliğinden değişmez) ve **Dosya > Seçenekler > Klavye** altında kendiniz ayarladığınız her kısayol da öyle: saklanmış bir kısayol asla değiştirilmez. 0.95'ten beri **Dosya > Seçenekler > Klavye** altındaki **Klavye Düzenimin Takımını Kullan…** düğmesi, şu anda kullanılan klavye düzenine uyan takımı uygular (bir Alman klavyesi için yukarıdaki tuşlar). Önce hangi standart kısayolların değişeceğini listeler; kendiniz ayarladığınız kısayollar kalır. Takım profilinize aittir. Sistem düzenini söylemiyorsa olağan küme kullanılır. (Linux: masaüstünün ya da X sunucusunun klavye ayarları; Windows: klavye düzeninin dili; Android: klavyenin dili.)

### Standart kısayollar

| Eylem | Kısayol |
|--------|----------|
| Yeni proje / proje aç | Ctrl+Shift+N / Ctrl+O |
| Kaydet / projeyi kapat / çık | Ctrl+S / Ctrl+W / Ctrl+Q |
| Yeni belge / yeni klasör | Ctrl+N / Alt+Shift+N |
| Yeniden adlandır / çoğalt | F2 / Ctrl+D |
| Öğeyi yukarı / aşağı / dışarı / içeri taşı | Ctrl+Alt+Up / Down / Left / Right |
| Seçimden böl / seçimi başlık yaparak böl | Ctrl+K / Ctrl+Shift+K |
| Belgeleri birleştir | Ctrl+Shift+M |
| Birleşik Metin / mantar pano / anahat | Ctrl+1 / Ctrl+2 / Ctrl+3 |
| Binder'ı / denetçiyi göster ya da gizle | Ctrl+Alt+B / Ctrl+Alt+I |
| Yazım modu / tam ekran | F11 / Ctrl+Shift+F11 |
| Binder'a odaklan / düzenleyiciye odaklan / diğer düzenleyici | Alt+1 / Alt+2 / F6 |
| Geri / ileri | Ctrl+[ / Ctrl+] (Alman düzeni: Alt+Shift+B / Alt+Shift+F) |
| Önceki / sonraki belge | Alt+Shift+Up / Alt+Shift+Down |
| Binder'da göster | Ctrl+Shift+R |
| Bul / değiştir / sonraki / önceki | Ctrl+F / Ctrl+H / F3 / Shift+F3 |
| Proje araması | Ctrl+Shift+F |
| Seçimi arama için kullan | Ctrl+E |
| Biçimlendirmeye göre bul / sonraki / önceki | Ctrl+F3 / Ctrl+G / Ctrl+Shift+G |
| Karalama defteri (eklenti) | Alt+Shift+Return; her programdan: bkz. Bölüm 10 |
| Ses ve video (eklenti): oynat / duraklat | Ctrl+Return |
| Ses ve video: geri atla / ileri atla | Ctrl+Alt+, / Ctrl+Alt+. |
| Ses ve video: zaman damgası ekle | Ctrl+Shift+Return |
| Mantar panoyu ya da anahatı filtrele | Ctrl+F (ikisinden birinde klavye varken) |
| Belgeye bağlantı | Ctrl+L |
| Yorum / dipnot | Shift+F2 / Ctrl+Shift+F2 |
| Satır içi açıklama / satır içi dipnot | Ctrl+Shift+A / Ctrl+Alt+F |
| Anlık görüntü al / başlıklı | Ctrl+5 / Ctrl+Shift+5 |
| Proje hedefleri / proje anahtar sözcükleri | Ctrl+Shift+T / Ctrl+Alt+K |
| Dosyaları içe aktar / derle | Ctrl+Shift+I / Ctrl+Shift+E |
| Geçerli belgeyi yazdır | Ctrl+P |
| Yazı tipleri penceresi / paragraf penceresi | Ctrl+T / Ctrl+Alt+P |
| Yazı tipi boyutunu büyüt / küçült | Ctrl+> / Ctrl+< |
| Biçimlendirmeyi kopyala / biçimlendirmeyi yapıştır | Ctrl+Alt+C / Ctrl+Alt+V |
| Biçimlendirmeyle yapıştır / stile uyarak yapıştır | Ctrl+Alt+Shift+V / Ctrl+Shift+V |
| Stil Yok / Başlık 1 / Başlık 2 / Başlık 3 | Ctrl+Alt+0 / Ctrl+Alt+1 / Ctrl+Alt+2 / Ctrl+Alt+3 |
| Stil listesinin sonraki altı stili (Biçim > Stil > Stil Listesini Özelleştir) | Ctrl+Alt+4 ... Ctrl+Alt+9 |
| Özete git / belge notlarına git (Gezin > Denetle) | Ctrl+Alt+Y / Ctrl+Alt+N |
| Şimdi yedekle | Ctrl+Shift+B |
| Senaryo modu / senaryo öğeleri listesi | Ctrl+8 / Ctrl+\\ (Alman düzeni: Ctrl+8 / Ctrl+Shift+O) |
| Bir senaryoda: sonraki öğe / öğeyi değiştir | Enter / Tab (bkz. Bölüm 4, Senaryo Yazımı) |
| Yakınlaştır / uzaklaştır / yakınlaştırmayı sıfırla | Ctrl++ / Ctrl+- / Ctrl+0 |
| Farenin altındaki alanı yakınlaştır | Ctrl+fare tekerleği |
| Cetveli göster ya da gizle | Ctrl+Alt+R |
| Daktilo kaydırma | Meta+Ctrl+T |
| Seçenekler | Ctrl+, |
| Bu kılavuz | F1 |

---

## 20. Sorun giderme

### Çökme raporları ve sistem bilgisi

{{APP}} bir hata nedeniyle sona ererse, kapanmadan önce bilgisayarınıza bir **çökme raporu** yazar. Penceresi 30 saniye boyunca yanıt vermezse (donar; süre **Dosya > Seçenekler > Ek Özellikler > Çökme Raporları** altında ayarlanır), takıldığı yeri gösteren bir **donma raporu** yazar; toparlanırsa rapor ne kadar sonra olduğunu söyler. Hiçbir şey hiçbir yere gönderilmez. Bir çökmeden sonra ya da donmuş bir {{APP}} programını sonlandırmak zorunda kaldıktan sonra {{APP}} programını bir sonraki başlatışınızda, bir ileti ne olduğunu söyler. Bir dakika içinde sona erdiyse, ileti, neden proje olabilir diye **Son Proje Olmadan Başlat** seçeneğini sunar.

- **Yardım > Sorun Giderme ve Geliştirme > Çökme Raporları…** çökme ve donma raporlarını tarihleri ve hata türleriyle listeler ({{APP}} çalışmaya devam ettiğinde "freeze (recovered)"). **Panoya Kopyala** seçili olanı kopyalar; böylece {{APP}} ile ilgilenen kişiye bir iletiye yapıştırabilirsiniz; **Klasörü Aç** dosyaları gösterir; **Sil** ve **Tümünü Sil…** onları kaldırır. En yeni 20 rapor tutulur (**Dosya > Seçenekler > Ek Özellikler > Çökme Raporları** sayıyı değiştirir).
- **Dosya > Seçenekler > Genel > Uyarılar > Dahili Günlük Konsolunu Göster**, programın ve Qt'nin başlatıldığından beri iletilerini (son 1000), yenileri geldikçe gösterir; bir şey çalışmadığında ve hiçbir ileti nedenini söylemediğinde yararlıdır.
- **Yardım > Sorun Giderme ve Geliştirme > Sistem Bilgisi…**, bir raporun taşıdığı program ve bilgisayar özetinin aynısını gösterir. **Bakılacak şeyler** ile başlar: yakında soruna yol açabilecek şeyler; örneğin neredeyse dolu ya da salt okunur bir sürücü, bir ağ klasöründeki proje, projenin gerektirdiğinden daha az boş alanı olan bir yedek sürücüsü, az boş bellek, uzun bir otomatik kayıt süresi, kapatılmış otomatik yedekler ya da son çökme ve donma raporları. Sonra program sürümü, çalışma zamanı (AppImage'ın içindeki kitaplıklar kümesi, Qt'sinin çıkış günü ve ne kadar eski olduğuyla: eski bir çalışma zamanı, daha yeni bir AppImage'ın güvenlik düzeltmeleri getirebileceği anlamına gelir) ve kurulu güncellemeler ve eklentiler, işletim sistemi, masaüstü, ekranlar (0.93.1'den beri her birinde bir pencerenin kullanabileceği alanla), girdi aygıtları (klavye, fare, dokunmatik ekran, kalem) ve ekran klavyesinin gösterilip gösterilmediği (0.93.1), işlemci ve bellek, boş alanlarıyla klasörler, kaydetme ve yedekleme ayarları ve açık proje gelir. Bir çökme olmayan bir sorunu bildirirken **Panoya Kopyala** ya da **Farklı Kaydet…** kullanın.

Bir rapor dosyaları, klasörleri ve belgeleri ve hatadan kısa süre önce kullanılan komutları ve tuşları adlandırır (yazılan harfler yalnızca sayılır, asla kaydedilmez). Belgelerinizin hiçbir metnini içermez. Ana klasörünüz **~** olarak yazılır.

{{APP}} dışarıdan durdurulduysa ya da bilgisayar kapatılmadan kapatıldıysa rapor yoktur, ama bir sonraki başlangıç bunu da söyler. Her iki durumda da çalışmanız, bitişten birkaç saniye önceki son otomatik kayda kadar kaydedilmiştir; bir şey eksikse belgenin anlık görüntülerine ya da en yeni yedeğe bakın (Bölüm 11 ve 15).

**"Proje dosyası okunamıyor."** Proje dosyası (*YourTitle*{{FILE}}), örneğin kaydedilirken bilgisayarın çökmesiyle bozulduysa, {{APP}} onun tuttuğu kopyayı (projenin en son açıldığı zamandan) sunar: **Kopyayı Kullan**. Bozuk dosya yanında saklanır (*… .damaged-<tarih>*). Binder'da o zamandan beri yapılan değişiklikler (başlıklar, sıra, etiketler) eksiktir; metinlerin hepsi oradadır ve o zamandan beri yapılan belgeler Araştırma'daki **Recovered Documents** klasörüne, ilk satırlarıyla adlandırılarak konur.

**"Proje dosyası içinde bulunmasına izin verilmeyen karakterler içeriyordu."** 0.20'den önceki sürümler görünmez denetim karakterlerini (diğer programlardan bir başlığa ya da özete yapıştırılan) kaydedebiliyordu; bundan sonra proje artık açılmıyordu. {{APP}} projeyi açarken bunları kaldırır ve size söyler; başka hiçbir şey değiştirilmez. 0.20'den beri asla kaydedilmezler.

**Bir belge boş gösteriliyor ve "Bu belgenin dosyası okunamıyor" notu var.** Metin dosyası orada ama okunamıyor (izinleri değiştirilmiş ya da bulunduğu sürücü gitmiş). {{APP}} onu boş gösterir ve dosyanın üzerine hiçbir zaman yazılmaması için düzenlemenize izin vermez. Dosyanın izinlerini ya da sürücüyü denetleyin, sonra projeyi kapatıp yeniden açın.

**Page Down ya da Page Up tuşuna basmak programı kapattı.** Bu, 0.19'dan önceki sürümlerdeki bir hataydı; 0.19 ya da sonrasına güncelleyin. O zamana kadar fareyle ya da kaydırma çubuğuyla kaydırın.

**Çift tıkladığımda program başlamıyor.** Dosyaya sağ tıklayın > Özellikler > İzinler ve "Çalıştırılabilir" işaretini koyun (Dolphin'de: "Dosyanın program olarak çalıştırılmasına izin ver"). Sonra yeniden çift tıklayın.

**Dosya iletişim kutuları diğer KDE programlarından farklı görünüyor.** **Dosya > Seçenekler > Genel** altında **Masaüstünün dosya iletişim kutularını kullan** seçeneğini açın ve {{APP}} programını yeniden başlatın.

**Araç çubuğu simgeleri koyu ya da açık temada zor görülüyor.** {{APP}}, masaüstünüzün simgelerinin açık ya da koyu kümesini temaya uyacak şekilde seçer. Masaüstünde uygun bir küme yoksa kendi simgelerini kullanır. Tek tek simgeler hâlâ uymuyorsa, **Görünüm > Temalar** altında **Varsayılan (Masaüstünü Izle)** seçin ve renkleri masaüstünün kendi ayarlarında belirleyin.

**Renkleri değiştirdim ve temanınkini geri istiyorum.** **Dosya > Seçenekler > Dış Görünüm > Renkleri ve Yazı Tiplerini Temaya Sıfırla** ya da tek bir renk ya da yazı tipinin yanındaki **↺** düğmesi.

**Bir program güncellemesi başlamadı.** {{APP}} başarısız bir güncellemeyi kendiliğinden kapatır (Bölüm 17). Yeniden kurmayı deneyebilir ya da yerleşik sürümü kullanmaya devam edebilirsiniz.

**Bir derleme biçimi eksik.** **Dosya > Seçenekler > Uzantılar** bölümünü denetleyin: biçimin hangi programa ihtiyaç duyduğunu listeler. Onu Discover'dan kurun ve {{APP}} programını yeniden başlatın.

**Yazım denetimi dilimi bilmiyor.** İspanyolca, Fransızca, Brezilya Portekizcesi, İtalyanca, Rusça, Endonezce, Türkçe, Lehçe ve Felemenkçe için "<dil> yazım sözlüğü" eklentisini kurun (**Yardım > Eklentiler ve Güncellemeler > Güncelleme veya Eklenti Kur**; indirme sayfası bunları listeler), {{APP}}'i yeniden başlatın ve dili **Dosya > Seçenekler > Düzeltmeler** altında seçin (Yazım Denetimi sayfası). Başka bir dil için Discover'dan ilgili *hunspell* sözlük paketini kurun.

**Proje "başka bir pencerede ya da başka bir bilgisayarda açık görünüyor."** Bu, proje başka bir yerde açık olan eşitlenmiş bir klasördeyse ya da başka bir bilgisayarda bir çökmeden sonra olur. Başka hiçbir yerde açık olmadığından eminseniz **Evet** seçin.

**Alt+sürükle bir referans bölmesi açmıyor.** Bazı masaüstleri Alt+sürükle'yi pencereleri taşımak gibi kendi işleri için kullanır. Bunun yerine **Gezin > Aç > Referans Bölmesinde** komutunu ya da belgenin sağ tık menüsünü kullanın ya da tuşu masaüstünün pencere ayarlarında değiştirin.

**Eski bir program sürümüne döndükten sonra bir senaryo düzensiz düz metin.** 0.10'dan önceki sürümler senaryo öğelerini bilmez ve gösterdikleri bir senaryoyu sıradan metin olarak kaydeder. Yeniden güncel sürüme geçin (**Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi**). Metnin hepsi oradadır; düzeni geri almak için belgeyi daha önce yapılmış bir anlık görüntüden ya da yedekten geri yükleyin (Bölüm 11 ve 15) ya da paragraflara altbilgideki açılır menüyle öğelerini yeniden verin.

**Karalama defteri kısayolu diğer programlarda hiçbir şey yapmıyor.** **Dosya > Seçenekler > Klavye** altında ayarlanan kısayollar yalnızca {{APP}} içinde çalışır. Diğer programlar için masaüstünün komutu başlatması gerekir (Bölüm 10, *Her programdan*). AppImage'ı taşırsanız komutu yeniden girin (AppImage'ın yerini adlandırır).

**Karalama defterinin fotoğraf makinesi ekranı okuyamadığını söylüyor.** Wayland altında bunu yalnızca masaüstünün kendi ekran görüntüsü programı yapabilir. *Spectacle* kurun (`sudo apt install kde-spectacle`) ve yeniden deneyin.

**Düzenle > Yazım Araçları "Kurulu yazım aracı yok" diyor.** Araçlar eklentilerdir: 0.11 güncellemesinin paketini kurun ya da **Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi** altında açın.

**Bir ses ya da video dosyası yalnızca "Varsayılan Uygulamayla Aç" sunuyor.** {{APP}} içinde oynatmak için *Media Playback* eklentisi gerekir (0.12 güncellemesinin paketinde); onu kurun ya da **Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi** altında açın. Düğmenin altındaki satır neyin eksik olduğunu söyler.

**Bir video oynuyor ama ses yok ya da bir ses dosyası "No sound output was found" diyor.** {{APP}} sesi masaüstünün ses sunucusu (PipeWire ya da PulseAudio) üzerinden oynatır. Sesin diğer programlarda çalıştığını ve masaüstünün ses ayarlarında doğru çıkışın seçildiğini denetleyin; {{APP}} çıkıştaki bir değişikliği hemen izler.

**"This file cannot be played here: it uses H.264 and AAC ..."** Dosya, *Media Playback* eklentisinin dışarıda bıraktığı, patentlerle lisanslı bir biçimdedir. **Extra Video and Audio Formats** eklentisini ya da onu *Media Playback* ile birlikte getiren **media-all-formats.qbpkg** paketini kurun (ikisi de proje sayfasının Sürümler sayfasında, Bölüm 1) ve {{APP}} programını yeniden başlatın.

**Bir dosya oynamıyor ya da resimsiz oynuyor.** Bazı türler yerleşik kod çözmenin parçası değildir (AV1 video, WMA Pro, WMV 9). Dosyayı başka bir programla açın (oynatıcıya sağ tıklayın: **Varsayılan Uygulamayla Aç**) ya da başka bir biçime dönüştürün.

**Bir .doc dosyası metin olarak değil dosya olarak ekleniyor.** **Word 97–2003 İçe Aktarma** eklentisi kurulu değil ya da kapalı (**Yardım > Eklentiler ve Güncellemeler > Eklenti Yöneticisi**); kurduktan sonra {{APP}} programını yeniden başlatın. İçe aktarmadan sonraki bir ileti eklentinin nelerin dışarıda bıraktığını adlandırır. Word 6.0 ve 95 dosyaları LibreOffice gerektirir (`sudo apt install libreoffice-writer`).

**Bir web sayfası boş ya da yanlış bölümle geliyor.** Tüm sayfayı almak ve gerek duymadığınızı silmek için Web Sayfası penceresinde **Yalnızca makale** işaretini kaldırın. Oturum açmayı gerektiren ya da tamamen betiklerle oluşturulan sayfalar bir tarayıcı olmadan okunamaz: sayfayı tarayıcınızda açın, metni seçin ve Binder'a sürükleyin (yeni bir belge olur). "Secure web pages cannot be fetched" OpenSSL 3'ün eksik olduğu (`sudo apt install libssl3`, her Ubuntu'nun parçasıdır) ya da eklentinin bozuk olduğu (yeniden kurun) anlamına gelir.

**Bir dosyanın ilk saniyesi sessiz ya da zaman önce hareketsiz kalıyor.** Ses çıkışının başlaması için bir an gerekir; bazı ses sunucularında bir saniyeden fazla. Başlangıcı kaçırdıysanız biraz geri sarın (Ctrl+Alt+,).

**Sayfa görünümündeki sayfalar basılı el yazmamla uyuşmuyor.** Uyuşmaları amaçlanmamıştır: sayfa görünümü siz yazarken metninizi sayfalar üzerinde gösterir. Bitmiş dosyanın sayfalarını derleme biçimi belirler (Bölüm 13).

**"Scrivener has this project open."** Scrivener projeyi açıkken ve bir çökmeden sonra *.scriv* içinde bir kilit dosyası bırakır. Projeyi Scrivener'da kapatın (her bilgisayarda) ve burada yeniden açın; Scrivener hiçbir yerde çalışmıyorsa **Evet** seçin.

**Scrivener biçimindeki bir proje Scrivener'daki en son değişikliklerimi göstermiyor.** {{APP}} *.scriv* dosyasını yalnızca proje açıldığında yeniden okur. Burada kapatın (**Dosya > Projeyi Kapat**), Scrivener ya da eşitleme hizmeti bitirsin ve yeniden açın.

**Ekle, Gezin, Proje, Belgeler ve Biçim menüleri gri.** Hiçbir proje açık değil. Birini açın ya da oluşturun (Dosya menüsü).

**Bir sorunu nereye bildirmeli.** **Yardım > Sorun Bildir…** projenin sorun sayfasını (https://github.com/Shelin-boop/Quillbinder_Collection/issues/new/choose) tarayıcınızda, bir sorun ve bir dilek için birer formla açar; bir GitHub hesabına ihtiyacınız vardır ve İngilizce ya da Almanca yazabilirsiniz. Sorunlar herkese açıktır: el yazmanızı ya da projenizi eklemeyin; **Yardım > Sorun Giderme ve Geliştirme > Sistem Bilgisi…** (**Panoya Kopyala**) bir raporun program ve bilgisayar hakkında ihtiyaç duyduğunu içerir. Bir **güvenlik sorunu** orada değil, projenin *SECURITY.md* dosyasında anlatıldığı gibi özel olarak bildirilir (deponun Security sekmesi, **Report a vulnerability**).

**Bir Word dosyasının içindekiler tablosunda sayfa numaraları yok.** Sayfa numaralarını metin işlemcisi alanı güncellediğinde doldurur; orada güncelleyin.

**Bir belgede bir şeyler ters gitti.** Denetçinin anlık görüntü sekmesine bakın ya da bir yedeği geri yükleyin (Bölüm 15).

## Katkıda bulunanlar ve lisans

- Baş Geliştirici: Claude.AI w. Opus 5.5 Extra

{{APP}}, Apache Lisansı 2.0 altında özgür bir yazılımdır: herkes, lisansı ve projeyi ve kaynağını adlandıran NOTICE dosyasını koruyarak, ücretli ya da ücretsiz olarak kullanabilir, kopyalayabilir, değiştirebilir ve iletebilir; garanti verilmez. 0.70.0'a kadar olan sürümler Zero-Clause BSD lisansı (0BSD) altında yayımlandı. Hakkında penceresi (Yardım menüsünde) sürümü ve lisansı gösterir.

{{APP}}, her biri kendi lisansı altında, başka projelerin parçalarını (Qt, Hunspell, ICU, Breeze simgeleri ve AppImage'ın içindeki diğer kitaplıklar) kullanır. **Yardım > Üçüncü Taraf Lisansları…** (Hakkında penceresinin bir düğmesi de) onları tam lisans metinleriyle listeler; aynı metinler AppImage'ın içinde, *usr/share/doc/quillbinder/licenses* altındadır. Her birinin, kullanılan sürümün tam olarak aynısı olan kaynak kodu, proje sayfasının **Third-party source code** sürümündedir (https://github.com/Shelin-boop/Quillbinder_Collection/releases/tag/third-party-sources); pencere her kitaplık için dosyayı adlandırır ve eklentilerin lisans notları kendilerininkiler için aynısını yapar.

Scrivener ve Scapple, Literature & Latte ürünlerinin adlarıdır. {{APP}} bağımsız bir programdır; Literature & Latte tarafından yapılmamıştır ve onunla bağlantılı değildir; projelerinizin programlar arasında taşınabilmesi için onların dosya biçimlerini okur ve yazar.

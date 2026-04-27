// Copyright (c) 2024 The NewYorkCoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// BIP39 mnemonic generation and seed derivation.
// Reference: https://github.com/bitcoin/bips/blob/master/bip-0039.mediawiki

#include <crypto/bip39.h>
#include <crypto/hmac_sha512.h>
#include <crypto/sha256.h>
#include <random.h>
#include <support/cleanse.h>

#include <algorithm>
#include <cstring>
#include <sstream>
#include <string>
#include <vector>

namespace bip39 {

// ---------------------------------------------------------------------------
// BIP39 English wordlist (2048 words, public domain)
// ---------------------------------------------------------------------------
static const char* const WORDLIST[2048] = {
    "abandon","ability","able","about","above","absent","absorb","abstract",
    "absurd","abuse","access","accident","account","accuse","achieve","acid",
    "acoustic","acquire","across","act","action","actor","actress","actual",
    "adapt","add","addict","address","adjust","admit","adult","advance",
    "advice","aerobic","afford","afraid","again","age","agent","agree",
    "ahead","aim","air","airport","aisle","alarm","album","alcohol",
    "alert","alien","all","alley","allow","almost","alone","alpha",
    "already","also","alter","always","amateur","amazing","among","amount",
    "amused","analyst","anchor","ancient","anger","angle","angry","animal",
    "ankle","announce","annual","another","answer","antenna","antique","anxiety",
    "any","apart","apology","appear","apple","approve","april","arch",
    "arctic","area","arena","argue","arm","armed","armor","army",
    "around","arrange","arrest","arrive","arrow","art","artefact","artist",
    "artwork","ask","aspect","assault","asset","assist","assume","asthma",
    "athlete","atom","attack","attend","attitude","attract","auction","audit",
    "august","aunt","author","auto","autumn","average","avocado","avoid",
    "awake","aware","away","awesome","awful","awkward","axis","baby",
    "balance","bamboo","banana","banner","bar","barely","bargain","barrel",
    "base","basic","basket","battle","beach","bean","beauty","because",
    "become","beef","before","begin","behave","behind","believe","below",
    "belt","bench","benefit","best","betray","better","between","beyond",
    "bicycle","bid","bike","bind","biology","bird","birth","bitter",
    "black","blade","blame","blanket","blast","bleak","bless","blind",
    "blood","blossom","blouse","blue","blur","blush","board","boat",
    "body","boil","bomb","bone","book","boost","border","boring",
    "borrow","boss","bottom","bounce","box","boy","bracket","brain",
    "brand","brave","breeze","brick","bridge","brief","bright","bring",
    "brisk","broccoli","broken","bronze","broom","brother","brown","brush",
    "bubble","buddy","budget","buffalo","build","bulb","bulk","bullet",
    "bundle","bunker","burden","burger","burst","bus","business","busy",
    "butter","buyer","buzz","cabbage","cabin","cable","cactus","cage",
    "cake","call","calm","camera","camp","can","canal","cancel",
    "candy","cannon","canvas","canyon","capable","capital","captain","car",
    "carbon","card","cargo","carpet","carry","cart","case","cash",
    "casino","castle","casual","cat","catalog","catch","category","cattle",
    "caught","cause","caution","cave","ceiling","celery","cement","census",
    "century","cereal","certain","chair","chalk","champion","change","chaos",
    "chapter","charge","chase","chat","cheap","check","cheese","chef",
    "cherry","chest","chicken","chief","child","chimney","choice","choose",
    "chronic","chuckle","chunk","cinnamon","circle","citizen","city","civil",
    "claim","clap","clarify","claw","clay","clean","clerk","clever",
    "click","client","cliff","climb","clinic","clip","clock","clog",
    "close","cloth","cloud","clown","club","clump","cluster","clutch",
    "coach","coast","coconut","code","coffee","coil","coin","collect",
    "color","column","combine","come","comfort","comic","common","company",
    "concert","conduct","confirm","congress","connect","consider","control","convince",
    "cook","cool","copper","copy","coral","core","corn","correct",
    "cost","cotton","couch","country","couple","course","cousin","cover",
    "coyote","crack","cradle","craft","cram","crane","crash","crater",
    "crawl","crazy","cream","credit","creek","crew","cricket","crime",
    "crisp","critic","cross","crouch","crowd","crucial","cruel","cruise",
    "crumble","crunch","crush","cry","crystal","cube","culture","cup",
    "cupboard","curious","current","curtain","curve","cushion","custom","cute",
    "cycle","dad","damage","damp","dance","danger","daring","dash",
    "daughter","dawn","day","deal","debate","debris","decade","december",
    "decide","decline","decorate","decrease","deer","defense","define","defy",
    "degree","delay","deliver","demand","demise","denial","dentist","deny",
    "depart","depend","deposit","depth","deputy","derive","describe","desert",
    "design","desk","despair","destroy","detail","detect","develop","device",
    "devote","diagram","dial","diamond","diary","dice","diesel","diet",
    "differ","digital","dignity","dilemma","dinner","dinosaur","direct","dirt",
    "disagree","discover","disease","dish","dismiss","disorder","display","distance",
    "divert","divide","divorce","dizzy","doctor","document","dog","doll",
    "dolphin","domain","donate","donkey","donor","door","dose","double",
    "dove","draft","dragon","drama","drastic","draw","dream","dress",
    "drift","drill","drink","drip","drive","drop","drum","dry",
    "duck","dumb","dune","during","dust","dutch","duty","dwarf",
    "dynamic","eager","eagle","early","earn","earth","easily","east",
    "easy","echo","ecology","economy","edge","edit","educate","effort",
    "egg","eight","either","elbow","elder","electric","elegant","element",
    "elephant","elevator","elite","else","embark","embody","embrace","emerge",
    "emotion","employ","empower","empty","enable","enact","endless","endorse",
    "enemy","energy","enforce","engage","engine","enhance","enjoy","enlist",
    "enough","enrich","enroll","ensure","enter","entire","entry","envelope",
    "episode","equal","equip","erase","erode","erosion","error","erupt",
    "escape","essay","essence","estate","eternal","ethics","evidence","evil",
    "evoke","evolve","exact","example","excess","exchange","excite","exclude",
    "exercise","exhaust","exhibit","exile","exist","exit","exotic","expand",
    "expire","explain","expose","express","extend","extra","eye","fable",
    "face","faculty","faint","faith","fall","false","fame","family",
    "famous","fan","fancy","fantasy","far","fashion","fat","fatal",
    "father","fatigue","fault","favorite","feature","february","federal","fee",
    "feed","feel","feet","fellow","felt","fence","festival","fetch",
    "fever","few","fiber","fiction","field","figure","file","film",
    "filter","final","find","fine","finger","finish","fire","firm",
    "first","fiscal","fish","fit","fitness","fix","flag","flame",
    "flash","flat","flavor","flee","flight","flip","float","flock",
    "floor","flower","fluid","flush","fly","foam","focus","fog",
    "foil","follow","food","foot","force","forest","forget","fork",
    "fortune","forum","forward","fossil","foster","found","fox","fragile",
    "frame","frequent","fresh","friend","fringe","frog","front","frost",
    "frown","frozen","fruit","fuel","fun","funny","furnace","fury",
    "future","gadget","gain","galaxy","gallery","game","gap","garbage",
    "garden","garlic","garment","gas","gasp","gate","gather","gauge",
    "gaze","general","genius","genre","gentle","genuine","gesture","ghost",
    "gift","giggle","ginger","giraffe","girl","give","glad","glance",
    "glare","glass","glide","glimpse","globe","gloom","glory","glove",
    "glow","glue","goat","goddess","gold","good","goose","gorilla",
    "gospel","gossip","govern","gown","grab","grace","grain","grant",
    "grape","grasp","grass","gravity","great","green","grid","grief",
    "grit","grocery","group","grow","grunt","guard","guide","guilt",
    "guitar","gun","gym","habit","hair","half","hammer","hamster",
    "hand","happy","harsh","harvest","hat","have","hawk","hazard",
    "head","health","heart","heavy","hedgehog","height","hello","helmet",
    "help","hen","hero","hidden","high","hill","hint","hip",
    "hire","history","hobby","hockey","hold","hole","holiday","hollow",
    "home","honey","hood","hope","horn","hospital","host","hour",
    "hover","hub","huge","human","humble","humor","hundred","hungry",
    "hunt","hurdle","hurry","hurt","husband","hybrid","ice","icon",
    "ignore","ill","illegal","image","imitate","immense","immune","impact",
    "impose","improve","impulse","inbox","income","increase","index","indicate",
    "indoor","industry","infant","inflict","inform","inhale","inject","inner",
    "innocent","input","inquiry","insane","insect","inside","inspire","install",
    "intact","interest","into","invest","invite","involve","iron","island",
    "isolate","issue","item","ivory","jacket","jaguar","jar","jazz",
    "jealous","jeans","jelly","jewel","job","join","joke","journey",
    "joy","judge","juice","jump","jungle","junior","junk","just",
    "kangaroo","keen","keep","ketchup","key","kick","kid","kingdom",
    "kiss","kit","kitchen","kite","kitten","kiwi","knee","knife",
    "knock","know","lab","ladder","lamp","language","laptop","large",
    "later","laugh","laundry","lava","law","lawn","lawsuit","layer",
    "lazy","leader","learn","leave","lecture","left","leg","legal",
    "legend","lemon","lend","length","lens","leopard","lesson","letter",
    "level","liar","liberty","library","license","life","lift","like",
    "limb","limit","link","lion","liquid","list","little","live",
    "lizard","load","loan","lobster","local","lock","logic","lonely",
    "long","loop","lottery","loud","lounge","love","loyal","lucky",
    "luggage","lumber","lunar","lunch","luxury","mad","magic","magnet",
    "maid","main","mammal","mango","mansion","manual","maple","marble",
    "march","margin","marina","market","marriage","mask","master","match",
    "material","math","matrix","matter","maximum","maze","meadow","mean",
    "medal","media","melody","melt","member","memory","mention","menu",
    "mercy","merge","merit","merry","mesh","message","metal","method",
    "middle","midnight","milk","million","mimic","mind","minimum","minor",
    "minute","miracle","miss","mitten","model","modify","mom","monitor",
    "monkey","monster","month","moon","moral","more","morning","mosquito",
    "mother","motion","motor","mountain","mouse","move","movie","much",
    "muffin","mule","multiply","muscle","museum","mushroom","music","must",
    "mutual","myself","mystery","naive","name","napkin","narrow","nasty",
    "natural","nature","near","neck","need","negative","neglect","neither",
    "nephew","nerve","nest","network","news","next","nice","night",
    "noble","noise","nominee","noodle","normal","north","notable","note",
    "nothing","notice","novel","now","nuclear","number","nurse","nut",
    "oak","obey","object","oblige","obscure","obtain","ocean","october",
    "odor","off","offer","office","often","oil","okay","old",
    "olive","olympic","omit","once","onion","open","opera","oppose",
    "option","orange","orbit","orchard","order","ordinary","organ","orient",
    "original","orphan","ostrich","other","outdoor","outside","oval","over",
    "own","oyster","ozone","pact","paddle","page","pair","palace",
    "palm","panda","panel","panic","panther","paper","parade","parent",
    "park","parrot","party","pass","patch","path","patrol","pause",
    "pave","payment","peace","peanut","pear","peasant","pelican","pen",
    "penalty","pencil","people","pepper","perfect","permit","person","pet",
    "phone","photo","phrase","physical","piano","picnic","picture","piece",
    "pig","pigeon","pill","pilot","pink","pioneer","pipe","pistol",
    "pitch","pizza","place","planet","plastic","plate","play","please",
    "pledge","pluck","plug","plunge","poem","poet","point","polar",
    "pole","police","pond","pony","pool","popular","portion","position",
    "possible","post","potato","pottery","poverty","powder","power","practice",
    "praise","predict","prefer","prepare","present","pretty","prevent","price",
    "pride","primary","print","priority","prison","private","prize","problem",
    "process","produce","profit","program","project","promote","proof","property",
    "prosper","protect","proud","provide","public","pudding","pull","pulp",
    "pulse","pumpkin","punish","pupil","purchase","purity","purpose","push",
    "put","puzzle","pyramid","quality","quantum","quarter","question","quick",
    "quit","quiz","quote","rabbit","raccoon","race","rack","radar",
    "radio","rage","rail","rain","raise","rally","ramp","ranch",
    "random","range","rapid","rare","rate","rather","raven","reach",
    "ready","real","reason","rebel","rebuild","recall","receive","recipe",
    "record","recycle","reduce","reflect","reform","refuse","region","regret",
    "regular","reject","relax","release","relief","rely","remain","remember",
    "remind","remove","render","renew","rent","reopen","repair","repeat",
    "replace","report","require","rescue","resemble","resist","resource","response",
    "result","retire","retreat","return","reunion","reveal","review","reward",
    "rhythm","ribbon","rice","rich","ride","ridge","rifle","right",
    "rigid","ring","riot","ripple","risk","ritual","rival","river",
    "road","roast","robot","robust","rocket","romance","roof","rookie",
    "rose","rotate","rough","royal","rubber","rude","rug","rule",
    "run","runway","rural","sad","saddle","sadness","safe","sail",
    "salad","salmon","salon","salt","salute","same","sample","sand",
    "satisfy","satoshi","sauce","sausage","save","say","scale","scan",
    "scatter","scene","scheme","scissors","scorpion","scout","scrap","screen",
    "script","scrub","sea","search","season","seat","second","secret",
    "section","security","seek","segment","select","sell","seminar","senior",
    "sense","sentence","series","service","session","settle","setup","seven",
    "shadow","shaft","shallow","share","shed","shell","sheriff","shield",
    "shift","shine","ship","shiver","shock","shoe","shoot","shop",
    "short","shoulder","shove","shrimp","shrug","shuffle","shy","sibling",
    "siege","sight","sign","silent","silk","silly","silver","similar",
    "simple","since","sing","siren","sister","situate","six","size",
    "sketch","skill","skin","skirt","skull","slab","slam","sleep",
    "slender","slice","slide","slight","slim","slogan","slot","slow",
    "slush","small","smart","smile","smoke","smooth","snack","snake",
    "snap","sniff","snow","soap","soccer","social","sock","solar",
    "soldier","solid","solution","solve","someone","song","soon","sorry",
    "soul","sound","soup","source","south","space","spare","spatial",
    "spawn","speak","special","speed","sphere","spice","spider","spike",
    "spin","spirit","split","spoil","sponsor","spoon","spray","spread",
    "spring","spy","square","squeeze","squirrel","stable","stadium","staff",
    "stage","stairs","stamp","stand","start","state","stay","steak",
    "steel","stem","step","stereo","stick","still","sting","stock",
    "stomach","stone","stop","store","storm","story","stove","strategy",
    "street","strike","strong","struggle","student","stuff","stumble","subject",
    "submit","subway","success","such","sudden","suffer","sugar","suggest",
    "suit","summer","sun","sunny","sunset","super","supply","supreme",
    "sure","surface","surge","surprise","sustain","swallow","swamp","swap",
    "swear","sweet","swift","swim","swing","switch","sword","symbol",
    "symptom","syrup","table","tackle","tag","tail","talent","tank",
    "tape","target","task","tattoo","taxi","teach","team","tell",
    "ten","tenant","tennis","tent","term","test","text","thank",
    "that","theme","then","theory","there","they","thing","this",
    "thought","three","thrive","throw","thumb","thunder","ticket","tilt",
    "timber","time","tiny","tip","tired","title","toast","tobacco",
    "today","together","toilet","token","tomato","tomorrow","tone","tongue",
    "tonight","tool","topic","topple","torch","tornado","tortoise","toss",
    "total","tourist","toward","tower","town","toy","track","trade",
    "traffic","tragic","train","transfer","trap","trash","travel","tray",
    "treat","tree","trend","trial","tribe","trick","trigger","trim",
    "trip","trophy","trouble","truck","truly","trumpet","trust","truth",
    "try","tube","tuition","tumble","tuna","tunnel","turkey","turn",
    "turtle","twelve","twenty","twice","twin","twist","two","type",
    "typical","ugly","umbrella","unable","unaware","uncle","uncover","under",
    "undo","unfair","unfold","unhappy","uniform","unique","universe","unknown",
    "unlock","until","unusual","unveil","update","upgrade","uphold","upon",
    "upper","upset","urban","useful","useless","usual","utility","vacant",
    "vacuum","vague","valid","valley","valve","van","vanish","vapor",
    "various","vast","vault","vehicle","velvet","vendor","venture","venue",
    "verb","verify","version","very","veteran","viable","vibrant","vicious",
    "victory","video","view","village","vintage","violin","virtual","virus",
    "visa","visit","visual","vital","vivid","vocal","voice","void",
    "volcano","volume","vote","voyage","wage","wagon","wait","walk",
    "wall","walnut","want","warfare","warm","warrior","waste","water",
    "wave","way","wealth","weapon","wear","weasel","weather","web",
    "wedding","weekend","weird","welcome","well","west","wet","what",
    "wheat","wheel","when","where","whip","whisper","wide","width",
    "wife","wild","will","win","window","wine","wing","wink",
    "winner","winter","wire","wisdom","wise","wish","witness","wolf",
    "woman","wonder","wood","wool","word","world","worry","worth",
    "wrap","wreck","wrestle","wrist","write","wrong","yard","year",
    "yellow","you","young","youth","zebra","zero","zone","zoo"
};

// ---------------------------------------------------------------------------
// Internal helpers
// ---------------------------------------------------------------------------

/** Split a string by spaces into a vector of tokens. */
static std::vector<std::string> SplitWords(const std::string& s)
{
    std::vector<std::string> out;
    std::istringstream ss(s);
    std::string word;
    while (ss >> word) {
        out.push_back(word);
    }
    return out;
}

/** Find the index of a word in WORDLIST. Returns -1 if not found. */
static int FindWord(const std::string& word)
{
    for (int i = 0; i < 2048; ++i) {
        if (word == WORDLIST[i]) return i;
    }
    return -1;
}

/**
 * PBKDF2-HMAC-SHA512 with exactly one output block (64 bytes).
 *
 * This is the BIP39 KDF: password=mnemonic, salt="mnemonic"+passphrase,
 * iterations=2048, dkLen=64.
 *
 * Algorithm (single block, i=1):
 *   U_1 = HMAC-SHA512(password, salt || 0x00000001)
 *   U_k = HMAC-SHA512(password, U_{k-1})  for k=2..c
 *   T_1 = U_1 XOR U_2 XOR ... XOR U_c
 *   DK  = T_1
 */
static void PBKDF2_HMAC_SHA512(
    const uint8_t* password, size_t password_len,
    const uint8_t* salt,     size_t salt_len,
    uint32_t       iterations,
    uint8_t*       out,      size_t out_len)
{
    // We only need one 64-byte block (dkLen=64, hLen=64)
    static const size_t BLOCK_SIZE = CHMAC_SHA512::OUTPUT_SIZE; // 64
    uint8_t T[BLOCK_SIZE];
    uint8_t U[BLOCK_SIZE];

    // Compute U_1 = HMAC-SHA512(password, salt || INT32_BE(1))
    {
        uint8_t block_num[4] = {0, 0, 0, 1};
        CHMAC_SHA512 hmac(password, password_len);
        hmac.Write(salt, salt_len);
        hmac.Write(block_num, 4);
        hmac.Finalize(U);
    }
    memcpy(T, U, BLOCK_SIZE);

    // Compute U_2 .. U_c, XOR into T
    for (uint32_t iter = 1; iter < iterations; ++iter) {
        CHMAC_SHA512 hmac(password, password_len);
        hmac.Write(U, BLOCK_SIZE);
        hmac.Finalize(U);
        for (size_t j = 0; j < BLOCK_SIZE; ++j) T[j] ^= U[j];
    }

    size_t copy_len = (out_len < BLOCK_SIZE) ? out_len : BLOCK_SIZE;
    memcpy(out, T, copy_len);
    memory_cleanse(T, sizeof(T));
    memory_cleanse(U, sizeof(U));
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

std::string GenerateMnemonic(int words)
{
    // Only 12 (128-bit) and 24 (256-bit) word mnemonics are supported
    if (words != 12 && words != 24) words = 12;

    int entropy_bits = (words == 24) ? 256 : 128;
    int entropy_bytes = entropy_bits / 8;

    // Generate cryptographically secure entropy
    std::vector<uint8_t> entropy(entropy_bytes);
    GetRandBytes(entropy.data(), entropy_bytes);

    // Compute SHA256 checksum: take first (entropy_bits / 32) bits = 4 or 8 bits
    uint8_t hash[CSHA256::OUTPUT_SIZE];
    CSHA256().Write(entropy.data(), entropy_bytes).Finalize(hash);

    // Build bit string: entropy bits + checksum bits
    // Total bits = entropy_bits + entropy_bits/32 = 11 * words
    int total_bits = entropy_bits + entropy_bits / 32;
    // Concatenate entropy and the first checksum byte into a bit buffer
    std::vector<uint8_t> bits_src(entropy.begin(), entropy.end());
    bits_src.push_back(hash[0]); // only need first byte for checksum

    // Extract 11-bit groups to get word indices
    std::string result;
    for (int i = 0; i < words; ++i) {
        int bit_start = i * 11;
        int idx = 0;
        for (int b = 0; b < 11; ++b) {
            int bit_pos = bit_start + b;
            int byte_pos = bit_pos / 8;
            int bit_in_byte = 7 - (bit_pos % 8);
            if ((bits_src[byte_pos] >> bit_in_byte) & 1) {
                idx |= (1 << (10 - b));
            }
        }
        if (i > 0) result += ' ';
        result += WORDLIST[idx];
    }

    memory_cleanse(entropy.data(), entropy.size());
    memory_cleanse(hash, sizeof(hash));
    return result;
}

bool ValidateMnemonic(const std::string& mnemonic)
{
    std::vector<std::string> words = SplitWords(mnemonic);
    if (words.size() != 12 && words.size() != 24) return false;

    int num_words = (int)words.size();
    int entropy_bits = num_words * 11 - num_words / 3; // 128 or 256
    int entropy_bytes = entropy_bits / 8;
    int checksum_bits = num_words / 3; // 4 or 8

    // Look up all words and build bit array
    std::vector<int> indices(num_words);
    for (int i = 0; i < num_words; ++i) {
        indices[i] = FindWord(words[i]);
        if (indices[i] < 0) return false;
    }

    // Reconstruct the entropy + checksum bytes from 11-bit indices
    // total bits = 11 * num_words = entropy_bits + checksum_bits
    int total_bits = 11 * num_words;
    std::vector<uint8_t> bits_out((total_bits + 7) / 8, 0);

    for (int i = 0; i < num_words; ++i) {
        int bit_start = i * 11;
        for (int b = 0; b < 11; ++b) {
            int bit_pos = bit_start + b;
            int byte_pos = bit_pos / 8;
            int bit_in_byte = 7 - (bit_pos % 8);
            if ((indices[i] >> (10 - b)) & 1) {
                bits_out[byte_pos] |= (1 << bit_in_byte);
            }
        }
    }

    // The last checksum_bits bits of bits_out should match the first
    // checksum_bits bits of SHA256(entropy)
    std::vector<uint8_t> entropy(bits_out.begin(), bits_out.begin() + entropy_bytes);
    uint8_t hash[CSHA256::OUTPUT_SIZE];
    CSHA256().Write(entropy.data(), entropy_bytes).Finalize(hash);

    // Build expected checksum byte (the high checksum_bits bits of hash[0])
    uint8_t checksum_mask = (uint8_t)(0xFF << (8 - checksum_bits));
    uint8_t expected = hash[0] & checksum_mask;
    // The checksum is stored in the top bits of the byte at position entropy_bytes
    uint8_t actual = bits_out[entropy_bytes] & checksum_mask;

    memory_cleanse(entropy.data(), entropy.size());
    memory_cleanse(hash, sizeof(hash));
    return (actual == expected);
}

std::vector<uint8_t> MnemonicToSeed(const std::string& mnemonic,
                                    const std::string& passphrase)
{
    if (!ValidateMnemonic(mnemonic)) return {};

    // BIP39: salt = "mnemonic" + passphrase
    std::string salt_str = std::string("mnemonic") + passphrase;

    const uint8_t* pwd  = reinterpret_cast<const uint8_t*>(mnemonic.c_str());
    const uint8_t* salt = reinterpret_cast<const uint8_t*>(salt_str.c_str());

    std::vector<uint8_t> seed(64);
    PBKDF2_HMAC_SHA512(pwd,  mnemonic.size(),
                       salt, salt_str.size(),
                       2048,
                       seed.data(), 64);
    return seed;
}

} // namespace bip39

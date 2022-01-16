{
    jsonfile: {
        type: 'JsonDepoSource',
        name: '',
        data : {
            filename: std.extVar("depofile"), // "g4tuple-qsn-v1-fixed.json.bz2",
            model: "electrons",  // take "n" from depo as already in number of electrons
            scale: 1.0,           // multiply by "n"
        }
    },
    depos_tn: self.jsonfile.type + ":" + self.jsonfile.name,
}

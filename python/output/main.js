function main() {
    Java.perform(function () {
        Java.enumerateClassLoaders({
            onMatch: function(loader) {
                let msg = `[loader]:${loader}`;
                console.log(msg);
            },
            onComplete: function() {
                let msg = `find loader end`;
                console.log(msg);
            }
        });
    })
}

setImmediate(main);
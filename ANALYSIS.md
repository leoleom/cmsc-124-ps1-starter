# Analysis

### Pick 3 of the 10 categories. For each, pick a language that gives it to you for free and say what that language pays for it. "Python has dictionaries" isn't an answer. What does Python's dictionary cost in memory or in speed compared to what you built, and where would you notice?



### You wrote the tag check in dt_value_as_int by hand. Some languages don't let you. They make the tagged union a language construct, so the compiler writes the check for you, refuses to compile a read that skips it, and refuses to compile a set of cases that misses one. Rust's enum and match work this way, and so do ML's datatypes and Swift's enumerations with associated values. What does the C version let you do that a compiler enforcing the check wouldn't, and is any of it worth wanting?



### Your dt_map keeps insertion order separately from the hash buckets, which is memory spent on something no lookup uses. Argue the other side: describe a design that drops it, say what breaks, and say whether you'd ship it.

The map's hash buckets are what make lookups fast and strictly speaking, a map could get by with just its buckets and a count. The separate insertion-order array does not help with `get` or `put` at all. 

That said, without that array, `dt_map_key_at` would have to scan buckets and collision chains to find the nth key. That would still work, but it would no longer preserve insertion order. The result would be a different printout, and the order could shift unpredictably when keys are removed and reinserted. Additionally, the current interface and tests depend on a stable ordering, the order array is part of the contract.

I would not ship this change under the current interface and tests. I would reconsider it if in case to say that iteration order is unspecified, then the tradeoff could be reconsidered. At that point, the memory saved would have to be weighed against the slower traversal.


### Compare access after release with an allocation that remains unreleased at the driver's final check. What damage can each cause in a long-running server? How does that answer change for a command-line tool that exits in a second?

A use-after-free happens when code reads or writes memory after it has already been freed. At that point the memory may already belong to some other object, so the program could possibly read garbage or corrupt another structure. In a long-running service, this is especially bad because the freed cell may be reused already, and this bug can damage the wrong data at the wrong time. 

A leak is different. It keeps memory allocated even though it is no longer needed. The problem is not immediate corruption, it is that memory usage slowly grows. Over time, a long-running program can run out of memory or slow down badly. However, in a short command-line program, a leak is usually harmless because the OS frees everything when the process exits.

TThe main difference is timing. A dangling pointer can cause damage immediately, while a leak causes damage only after repeated use over time. That is why `DT_ERR_RELEASED` is treated as a checked failure on every operation, while `DT_ERR_LEAK` is only reported once, at shutdown, by the driver after it checks every reference. The system is treating the immediate danger of stale memory as more serious than gradual memory growth.
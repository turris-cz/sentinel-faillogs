bin_PROGRAMS += sentinel-faillogs

faillogs_sources = \
	%reldir%/config.h \
	%reldir%/config.c \
	%reldir%/log.h \
	%reldir%/log.c \
	%reldir%/logread.h \
	%reldir%/logread.c \
	%reldir%/parser.h \
	%reldir%/parser.c \
	%reldir%/sender.h \
	%reldir%/sender.c

sentinel_faillogs_SOURCES = \
	%reldir%/sentinel_faillogs.c \
	${faillogs_sources}

sentinel_faillogs_CFLAGS = \
	${logc_CFLAGS} \
	${libevent_CFLAGS} \
	${logc_event_CFLAGS} \
	${czmq_CFLAGS} \
	${logc_czmq_CFLAGS} \
	${msgpack_CFLAGS} \
	${libconfig_CFLAGS} \
	${CODE_COVERAGE_CFLAGS}
sentinel_faillogs_LDADD = \
	${logc_LIBS} \
	${libevent_LIBS} \
	${logc_event_LIBS} \
	${czmq_LIBS} \
	${logc_czmq_LIBS} \
	${msgpack_LIBS} \
	${libconfig_LIBS} \
	${CODE_COVERAGE_LIBS}

faillogs_gperf = %reldir%/identifier.gperf.c
$(sentinel_faillogs_OBJECTS): $(faillogs_gperf)
gperf_files += ${faillogs_gperf}

linted_files += $(sentinel_faillogs_SOURCES)

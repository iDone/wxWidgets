/////////////////////////////////////////////////////////////////////////////
// Name:        wx/secretstore.h
// Purpose:     wxSecretStore and related classes documentation
// Author:      Vadim Zeitlin
// Copyright:   (c) 2016 Vadim Zeitlin <vadim@wxwidgets.org>
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////


/**
    Represents the value of a secret in wxSecretStore.

    Immutable value-like class which tries to ensure that the secret value will
    be removed once it's not needed any more.

    @library{wxbase}
    @category{misc}

    @since 3.1.1
 */
class wxSecretValue
{
public:
    /**
        Creates an empty secret value (not the same as an empty password).
     */
    wxSecretValue();

    /**
        Creates a secret value from the given data.

        The @a data argument may contain NUL bytes and doesn't need to be
        NUL-terminated.

        Notice that at least under MSW the maximal size of the secret is
        limited. The exact limit depends on the OS version and is e.g. 2560 for
        Windows 7.
     */
    wxSecretValue(size_t size, const void *data);

    /**
        Creates a secret value from the given string.

        The @a secret argument may contain NUL bytes.

        The secret value will stored serialized in UTF-8 encoding.
     */
    explicit wxSecretValue(const wxString& secret);

    /**
        Creates a copy of an existing secret.
     */
    wxSecretValue(const wxSecretValue& other);

    /**
        Assigns another secret to this one.
     */
    wxSecretValue& operator=(const wxSecretValue& other);

    /**
        Wipes out the secret value from memory before destroying the object.

        This method doesn't provide any real security guarantee, but it does
        reduce the likelihood that secret value is leaked if the program
        crashes and ends in a core or a minidump file, for example.

        See Wipe() method if you need to overwrite another region of memory
        where the secret was copied to or from.
     */
    ~wxSecretValue();

    /**
        Check if a secret is not empty.
     */
    bool IsOk() const;

    /**
        Compare with another secret for equality.
     */
    bool operator==(const wxSecretValue& other) const;

    /**
        Compare with another secret for inequality.
     */
    bool operator!=(const wxSecretValue& other) const;

    /**
        Get the size, in bytes, of the secret data.

        May return 0.

        @see GetData()
     */
    size_t GetSize() const;

    /**
        Get read-only access to the secret data.

        Don't assume it is NUL-terminated, use GetSize() instead.

        @see GetAsString()
     */
    const void *GetData() const;

    /**
        Get the secret data as a string.

        This is a more convenient but less secure alternative to using
        GetSize() and GetData(), as this function creates another copy of a
        secret which won't be wiped when this object is destroyed and you will
        need to call WipeString() to overwrite the content of the returned
        string, as well all its copies, if any, manually to avoid the secret
        being left in memory.

        This function uses the specified @a conv object to convert binary
        secret data to string form. As the secret data may have been created
        by external programs not using wxWidgets API, it may be not a valid
        UTF-8-encoded string, so by default ::wxConvWhateverWorks, which tries
        to interpret it in any way not avoiding loss of data, is used. However
        if the secrets are only saved by the program itself and are known to be
        always encoded in UTF-8, it may be better to pass ::wxConvUTF8 as the
        converter to use.
     */
    wxString GetAsString(const wxMBConv& conv = wxConvWhateverWorks) const;

    /**
        Erase the given area of memory overwriting its presumably sensitive
        content.
     */
    static void Wipe(size_t size, void *data);

    /**
        Overwrite the contents of the given string.
     */
    static void WipeString(wxString& str);
};

/**
    A collection of secrets, sometimes called a key chain.

    This class provides access to the secrets stored in the OS-provided
    facility, e.g. credentials manager under MSW, keychain under OS X or
    Freedesktop-compliant password storage mechanism such as GNOME keyring
    under Unix systems.

    Currently only the access to the default keychain/ring is provided using
    GetDefault() method, support for other ones could be added in the future.
    After calling this method just call Save() to store a password entered by
    user and then call Load() to retrieve it during next program execution.
    See @ref page_samples_secretstore for an example of using this class.

    The @c service parameter of the methods in this class should describe the
    purpose of the password and be unique to your program, e.g. it could be
    "MyCompany/MyProgram/SomeServer". Note that the server name must be
    included in the string to allow storing passwords for more than one server.

    Notice that this class is always available under MSW (except when using
    MinGW32 which doesn't provide the required @c wincred.h header) and OS X
    but requires libsecret (see https://developer.gnome.org/libsecret/) under
    Unix and may not be compiled in if it wasn't found. You can check @c
    wxUSE_SECRETSTORE to test for this. Moreover, retrieving the default
    secret store may also fail under Unix during run-time if the desktop
    environment doesn't provide one, so don't forget to call IsOk() to check
    for this too.

    @library{wxbase}
    @category{misc}

    @since 3.1.1
 */
class wxSecretStore
{
public:
    /**
        Returns the default secrets collection to use.

        Call IsOk() on the returned object to check if this method succeeded.
     */
    static wxSecretStore GetDefault();

    /**
        Check if this object is valid.
     */
    bool IsOk() const;

    /**
        Store a secret.

        The service name should be user readable and unique.

        If a secret with the same service name and user already exists, it will
        be overwritten with the new value.

        Returns false after logging an error message if an error occurs,
        otherwise returns true indicating that the secret has been stored and
        can be retrieved by calling Load() later.
     */
    bool Save(const wxString& service,
              const wxString& user,
              const wxSecretValue& secret);

    /**
        Look up a secret.

        If no such secret is found, an empty value is returned, but no error is
        logged (however an error may still be logged if some other error
        occurs). If more than one secret matching the parameters exist, only
        one arbitrarily chosen of them is returned (notice that it's impossible
        to get into such situation using this API only).
     */
    wxSecretValue Load(const wxString& service, const wxString& user) const;

    /**
        Delete a previously stored secret.

        If there is more than one matching secret, all of them are deleted.

        If any secrets were deleted, returns true. Otherwise returns false and
        logs an error if any error other than not finding any matching secrets
        occurred.
     */
    bool Delete(const wxString& service, const wxString& user);
};

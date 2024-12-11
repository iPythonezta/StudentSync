import React, {useState, useContext, createContext} from "react";

const Context = createContext();

export default function ContextProvider ({children}) {
    const [token, setToken] = useState(null);
    const [login, setLogin] = useState(false);

    return (
        <Context.Provider value={{token, setToken, login, setLogin}}>
            {children}
        </Context.Provider>
    )
}

export const useContextApi = () => useContext(Context);

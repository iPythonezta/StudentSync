import {Container} from 'react-bootstrap';
import Login from './Components/Login';
import Home from './Components/Home';
import Users from './Components/Users';
import {BrowserRouter as Router, Route, Routes} from 'react-router-dom';
import {ToastContainer} from 'react-toastify';
import { useContextApi } from './ContexApi';
import { useEffect } from 'react';

import ContextProvider from './ContexApi';
import axios from 'axios';

import 'bootstrap/dist/css/bootstrap.min.css';
import 'react-toastify/dist/ReactToastify.css';
import './App.css';
function App() {
  const { userData, setUserData, token, setToken,login, setLogin } = useContextApi();
  useEffect(()=>{
    let tempTok = localStorage.getItem("token");
    setLogin(localStorage.getItem("token") != null);
    if (tempTok != null) {
        axios.defaults.headers.common['Authorization'] = `Bearer ${tempTok}`;
        axios.get("http://localhost:2028/api/user/").then((res) => {
            if (res.status == 200) {
                setUserData(res.data);
                setLogin(true);
                setToken(tempTok);
            }
        }).catch((error)=>{
          setLogin(false);
          localStorage.removeItem("token");
          setToken(null);
          setUserData(null);
          console.log(error);
        });
    }

  }, [token, login])
  return (
      <>
        <ToastContainer />
        <Router>
          <Routes>
            <Route path="/" element={<Home />} />
            <Route path="/login" element={<Login />} />
            <Route path="/users" element={<Users />} />
          </Routes>
        </Router>
      </>
  );
}

export default App;
